# 2022-12-06

## Malloc Lab 구현하기

### 2. `Next-fit` 구현하기

```c
static void *find_fit(size_t asize)
{
    char *bp = findp;
        for (bp = NEXT_BLKP(bp); GET_SIZE(HDRP(bp)) != 0; bp = NEXT_BLKP(bp))
        {
            if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
            {
                findp = bp;
                return bp;
            }
        }

        for (bp = heap_listp; bp < findp;)
        {
            bp = NEXT_BLKP(bp);
            if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
            {
                findp = bp;
                return bp;
            }
        }
        return NULL; /* No fit */
}
```

- 기존 `util` 44 + `thru` 10 = 54/100 에서  `util` 43 + `thru` 40 = 83/100으로 점수 상승
  - 교재에서 언급된 util 성적이 낮아지는 일부 테스트 케이스에 대해 고민해볼 것





### 3. Segregated free list 만들어보기

- 참고코드(진행중)
  - https://d-cron.tistory.com/34?category=1003412
  - https://velog.io/@gitddabong/week06.-malloc-lab-Segregated-list
