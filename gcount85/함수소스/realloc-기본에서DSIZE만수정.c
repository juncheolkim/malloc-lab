// DSIZE만 수정한 realloc 함수 
void *mm_realloc(void *oldptr, size_t size) // realloc할 포인터, 새롭게 필요한 총 사이즈
{
    // 포인터가 Null이 아니고 이미 free된 상태가 아닌 경우 (지워도 점수 영향X)
    if (oldptr != NULL && GET_ALLOC(HDRP(oldptr)))
    {
        size_t copySize; // 복사해야 할 기존 사이즈 선언

        void *newptr = mm_malloc(size);
        if (newptr == NULL)
            return NULL;

        copySize = GET_SIZE(HDRP(oldptr)) - DSIZE;

        if (size < copySize)
            copySize = size;

        memcpy(newptr, oldptr, copySize); // 새로운 포인터로, 올드 포인터에서 copysize 만큼 복사
        mm_free(oldptr);
        return newptr;
    }
    return NULL;
}