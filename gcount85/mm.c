/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * 한글 주석은 이은민이 작성함
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "team 2",
    /* First member's full name */
    "Juncheol Kim",
    /* First member's email address */
    "kimjoon889@gmail.com",
    /* Second member's full name (leave blank if none) */
    "Hyunhong Lee",
    /* Second member's email address (leave blank if none) */
    "hyunhong1012@gmail.com"};

/* 얼라인먼트(바이트 단위); 싱글워드는 4, 더블워드는 8 */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 책 보고 매크로 상수 추가한 부분: Basic constants and macros */
#define WSIZE 4             /* Word and header/footer size: 4바이트 */
#define DSIZE 8             /* Double word size: 8바이트 */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes): 1024개 워드 */

/*
1 << 12는 이진수 연산자인 왼쪽 시프트(left shift)를 사용한 것입니다.
왼쪽 시프트 연산은 왼쪽으로 이동한 후, 오른쪽의 빈 자리를 0으로 채우는 연산자입니다.
예를 들어, 1 << 12는 1이라는 수를 왼쪽으로 12번 이동시킨 결과인 값인 4096을 나타냅니다.
이진수로 변환하면 1은 0001이며, 왼쪽 시프트 연산을 하면 0001 << 12는 000000000001이 되고,
이것을 십진수로 변환하면 4096이 됩니다.
== 1024개의 워드!!!!!!!!! 128개의 더블 워드~~!!!
*/

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* 블록 사이즈와, 할당/가용 비트를 워드 안에 packing */
#define PACK(size, alloc) ((size) | (alloc))

/* 주소 p에 담긴 워드를 읽고 쓰기 */
#define GET(p) (*(unsigned int *)(p))              // 워드 읽기
#define PUT(p, val) (*(unsigned int *)(p) = (val)) // 워드 쓰기

/* 주소 p에 담긴 블록 사이즈와 할당/가용 비트를 읽기 */
// 사이즈 값을 읽기 위한 비트 연산 & 할당/가용 비트를 읽기 위한 비트 연산
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

// bp = 블록의 *데이터의 시작 부분을 가리키는 포인터

// 헤더는 = 데이터 영역 - 1워드
#define HDRP(bp) ((char *)(bp)-WSIZE)

// 풋터 = bp + 전체사이즈 - 더블워드(풋터,헤더)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* 블록을 가리키는 포인터 bp가 주어졌을 때, 다음 블럭과 이전 블럭의 주소를 계산 */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

// 프로토타입 선언
static void *coalesce(void *bp);
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);

// static 전역 변수 선언
static char *heap_listp; // 항상 힙의 프롤로그 블록을 가리킬 포인터 변수
static char *findp;      // next-fit을 위한 변수

/* 말록 패키지 초기화 */
int mm_init(void)
{
    // 4 워드를 가져와서 빈 가용리스트를 만들도록 초기화
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                            /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header:  */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2 * WSIZE);
    // 요기까지 초기화 코드

    /* CHUNKSIZE 바이트의 가용 블록으로 비어있는 힙을 확장함 */
    // 청크사이즈(4096바이트)/워드사이즈(4바이트) = 워드개수(1024바이트)
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    findp = (char *)heap_listp; // have to casting: heap_listp is void

    return 0;
}

// 새 가용 블록으로 힙을 늘리기
static void *extend_heap(size_t words)
{
    char *bp; // char 자료형을 쓰는 이유는 1바이트면 주소를 읽기 충분함
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE; // 워드를 2로 나눠서 홀수면 워드 한개 더 추가
    if ((long)(bp = mem_sbrk(size)) == -1)                    // sbrk 함수로부터 힙 못 받으면 NULL 반환
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    // 새로 받은 가용 블럭의 헤더/푸터 + 에필로그 헤더를 초기화
    // bp는 이전의 힙 메모리의 에필로그 블록 다음을 가리키게 됨
    PUT(HDRP(bp), PACK(size, 0)); /* Free block header, 할당비트 0 */
    PUT(FTRP(bp), PACK(size, 0)); /* Free block footer, 할당비트 0 */
    // 에필로그 헤더의 공간은 항상 보장되는지??? 푸터를 끝에 적으면 에필로그 블록 자리 없을 거 같은데???????
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header, 크기 0, 할당비트 1 */

    /* 이전의 힙이 가용 블록으로 끝난 경우, 연결시켜주기 위해 연결 함수 호출 */
    return coalesce(bp); // 통합된 블럭의 블럭 포인트 반환
}

/*
 * mm_malloc - brk 포인터를 증가시킴으로서 블록을 할당.
 * 항상 정렬의 배수의 사이즈를 가진 블럭을 할당함.
 */
void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size for 더블워드 정렬, 헤더/풋터*/
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* 블록 사이즈를 정렬 요건과 헤더/풋터를 위한 공간을 확보하기 위해 조절 */
    if (size <= DSIZE) // 만약 요청 크기가 8바이트(워드 2개)랑 같거나 작으면
    {
        asize = 2 * DSIZE; // 최소 16바이트 블록으로 줌 (워드 4개)
    }
    else // 요청 크기가 8바이트보다 크면(더블 워드 이상)
    {
        //  오버헤드 바이트를 추가하고, 가까운 8의 배수로 반올림
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) // fit하는 블록이 있을 때
    {
        place(bp, asize); // bp가 가리키는 블록에 해당 사이즈 할당
        findp = bp;       // next-fit 마지막 검색 지점 설정
        return bp;        // 포인터 반환
    }

    /* 맞는 가용 블럭이 없으면, 메모리를 더 받아서 힙을 새로운 가용 블럭으로 확장하고,
    요청한 블록을 새 가용블럭에 배치 */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize); // 필요한 경우 블록 분할
    findp = bp;       // next-fit 때문에 추가

    return bp;
}



// next-fit search
static void *find_fit(size_t asize)
{
    char *bp = findp;
    // 에필로그 블록에 도달할 때까지 블록 탐색
    for (bp = NEXT_BLKP(bp); GET_SIZE(HDRP(bp)) != 0; bp = NEXT_BLKP(bp))
    {
        // 가용 블록이고, 필요한 사이즈와 같거나 클 때 해당 블록 포인터 반환
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {
            findp = bp;
            return bp;
        }
    }

    // 에필로그 블록에 도달했는데 못 찾은 경우, 프롤로그 블록부터 다시 탐색
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

// 가용 블록의 시작 부분에 조절한 사이즈(asize)의 요청한 블록(bp)을 배치하고
// 헤더, 풋터 지정
// 나머지 부분의 크기가 최소 블록 크기와 같거나 큰 경우에만 분할함
static void place(void *bp, size_t asize)
{
    size_t fsize = GET_SIZE(HDRP(bp)); // 가용 블럭의 전체 사이즈

    // 가용 블럭의 사이즈와 배치하고자 사이즈의 차이가 워드 4개보다 같거나 크면 -> 분할 O
    if ((fsize - asize) >= (2 * DSIZE))
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(fsize - asize, 0));
        PUT(FTRP(bp), PACK(fsize - asize, 0));
    }

    // 가용 블럭의 사이즈와 배치하고자 사이즈의 차이가 워드 4개보다 작으면 -> 분할 X
    else
    {
        PUT(HDRP(bp), PACK(fsize, 1));
        PUT(FTRP(bp), PACK(fsize, 1));
    }
}

/*
 * mm_free - Freeing a block does nothing.
 * 블럭을 프리시키고 인접한 가용 블록과 경계 태그 연결을 이용함
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/* 경계 태그 연결하는 함수. 합쳐진 블락의 bp를 반환함.  */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    if (prev_alloc && next_alloc)
    {               /* Case 1 */
        findp = bp; // next-fit 코드 때문에 추가
        return bp;
    }

    else if (prev_alloc && !next_alloc)
    { /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc)
    { /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else
    { /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    findp = bp; // next-fit 코드 때문에 추가

    return bp;
}

// 인자로 받은 bp의 데이터를 새로운 사이즈의 블럭에 재할당
void *mm_realloc(void *bp, size_t size)
{
    size_t old_size = GET_SIZE(HDRP(bp));
    size_t new_size = size + DSIZE;

    if (new_size <= old_size)
    {
        return bp;
    }

    else
    {
        size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
        size_t current_size = old_size + GET_SIZE(HDRP(NEXT_BLKP(bp)));

        // next block이 Free이고, old, next block의 사이즈 합이 new_size보다 크면 바로 합쳐서 쓰기
        if (!next_alloc && current_size >= new_size)
        {
            PUT(HDRP(bp), PACK(current_size, 1));
            PUT(FTRP(bp), PACK(current_size, 1));
            return bp;
        }

        // 아닌 경우 새로운 block malloc으로 할당 받아 데이터 복사
        else
        {
            void *new_bp = mm_malloc(new_size);
            place(new_bp, new_size);
            memcpy(new_bp, bp, new_size); // old_bp로부터 new_size만큼 new_bp로 복사
            mm_free(bp);
            return new_bp;
        }
    }
}






