
// realloc 사이즈 비교 + 그 자리에 place 
void *mm_realloc(void *oldptr, size_t size) // realloc할 포인터, 새롭게 필요한 총 사이즈
{
    // 포인터가 Null이 아니고 이미 free된 상태가 아닌 경우 (지워도 점수 영향X)
    if (oldptr != NULL && GET_ALLOC(HDRP(oldptr)))
    {
        size_t copySize; // 복사해야 할 기존 사이즈 선언
        size_t needSize = (size + DSIZE);
        size_t nowSize = GET_SIZE(HDRP(oldptr));

        // 블록 사이즈가 잘 맞으면 재할당 X
        if (needSize == nowSize)
        {
            return oldptr;
        }

        // 블록 사이즈가 할당 요청하는 사이즈보다 작으면
        if (needSize < nowSize)
        {
            size_t subSize = nowSize - needSize; // 사이즈의 차이 계산
            if (subSize >= 2 * DSIZE)            // 사이즈의 차이가 더블 워드보다 큼 -> 분할 가능
            {
                void *bp2 = FTRP(oldptr) - (subSize - DSIZE);
                PUT(FTRP(oldptr), PACK(subSize, 1));         // 기존 풋터를 뉴 풋터로 갱신
                PUT(HDRP(oldptr), PACK(needSize, 1));        // 기존 헤더의 사이즈, 비트 갱신
                PUT(FTRP(oldptr), PACK(needSize, 1));        // 기존 블럭의 풋터를 새로 만듦
                PUT(FTRP(oldptr) + WSIZE, PACK(subSize, 1)); // 남은 블럭의 헤더 갱신
                mm_free(bp2);
            }
            else // 사이즈의 차이가 더블 워드보다 작음 -> 분할 못함, 기존 것만 place
            {
                PUT(HDRP(oldptr), PACK(needSize, 1)); // 기존 헤더의 사이즈, 비트 갱신
                PUT(FTRP(oldptr), PACK(needSize, 1)); // 기존 풋터를 뉴 풋터로 갱신
            }
            return oldptr;
        }

        else // needSize > nowSize: 필요한 크기가 지금 크기보다 크면
        {
            void *newptr = mm_malloc(size);
            if (newptr == NULL)
                return NULL;

            copySize = GET_SIZE(HDRP(oldptr)) - DSIZE;

            memcpy(newptr, oldptr, copySize); // 새로운 포인터로, 올드 포인터에서 copysize 만큼 복사
            mm_free(oldptr);
            return newptr;
        }
    }
    return NULL;
}
