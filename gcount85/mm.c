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

/* 블록을 가리키는 포인터 bp가 주어졌을 때, 그것의 헤더와 풋터의 주소를 계산 */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* 블록을 가리키는 포인터 bp가 주어졌을 때, 다음 블럭과 이전 블럭의 주소를 계산 */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

// 프로토타입 선언
static void *coalesce(void *bp);
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
// static void printblock(void *bp);

static char *heap_listp; // 항상 힙의 프롤로그 블록을 가리킬 포인터 변수

//

/*
 * mm_init - initialize the malloc package.
 */

int mm_init(void)
{

    // 책보고 추가한 부분
    /* Create the initial empty heap */

    // 4 워드를 가져와서 빈 가용리스트를 만들도록 초기화
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                            /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header:  */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2 * WSIZE);
    // 요기까지 초기화 코드

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    // 청크사이즈(4096바이트)/워드사이즈(4바이트) = 워드개수(1024바이트)
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

// 책보고 추가
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
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 * size 바이트의 메모리 블록을 요청
 *
 */

// 기존 코드에 있던 부분
// void *mm_malloc(size_t size)
// {
//     int newsize = ALIGN(size + SIZE_T_SIZE);
//     void *p = mem_sbrk(newsize);
//     if (p == (void *)-1)
//         return NULL;
//     else
//     {
//         *(size_t *)p = size;
//         return (void *)((char *)p + SIZE_T_SIZE);
//     }
// }

// mm_malloc: 책에서 복붙한 부분
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
        return bp;        // 포인터 반환
    }

    /* 맞는 가용 블럭이 없으면, 메모리를 더 받아서 힙을 새로운 가용 블럭으로 확장하고,
    요청한 블록을 새 가용블럭에 배치 */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize); // 필요한 경우 블록 분할
    return bp;
}

// 묵시적 가용 리스트에서 first fit 검색을 수행하는 함수
static void *find_fit(size_t asize)
{
    /* First-fit search */
    void *bp;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {
            return bp;
        }
    }
    return NULL; /* 맞는 fit이 없는 경우 */
    // #endif // <- 얘는 책에 있었는데 #ifdef랑 같이 필요함. 왜 얘만 혼자 있는 건지 모르겠음.
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
    { /* Case 1 */
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
    return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 * 에러메시지: mm_realloc did not preserve the data from old block
 */
void *mm_realloc(void *ptr, size_t size) // realloc할 포인터, 새롭게 필요한 총 사이즈
{
    void *oldptr = ptr; // 기존 포인터
    void *newptr;       // 새로운 포인터 선언
    size_t copySize;    // 복사해야 할 기존 사이즈 선언

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;

    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);

    // 이 경우에는 말록으로 새 포인터를 받을 필요 없을 듯 한데? 남은 부분만 free 시키면 되지 않나
    // -> 코치님이 말하시길 메모리 단편화와 시간을 따져봐야 할 문제 !
    if (size < copySize)
        copySize = size;

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

// 새롭게 구한 realloc 함수: 65점
/* mm_realloc - Reallocate a block of memory */
void *mm_realloc(void *ptr, size_t size)
{
    // // Check if the pointer is valid and not already freed
    // if (ptr != NULL && !is_freed(ptr))
    // {
    // Allocate a new block of memory
    void *new_ptr = mm_malloc(size);
    if (new_ptr != NULL)
    {
        // Copy the data from the old block to the new one
        memcpy(new_ptr, ptr, size);
        // Free the old block
        mm_free(ptr);
        // Return a pointer to the newly allocated block
        return new_ptr;
    }
    // }
    // return NULL;
}

// 복잡한 realloc 함수
// void *mm_realloc(void *ptr, size_t size)
// {
//     // if ptr is NULL, then it is equivalent to malloc
//     if (ptr == NULL)
//     {
//         return mm_malloc(size);
//     }

//     // if size is 0, then free the memory previously allocated
//     if (size == 0)
//     {
//         mm_free(ptr);
//         return NULL;
//     }

//     // get the header of the block
//     void *header_ptr = HDRP(ptr);
//     void *next_header_ptr = NEXT_BLKP(header_ptr);

//     // get the size of the current block
//     size_t block_size = GET_SIZE(HDRP(header_ptr));

//     // if the block fits perfectly, no need to reallocate
//     if (block_size == size)
//     {
//         return ptr;
//     }
//     // if the block is smaller than the size requested
//     else if (block_size < size)
//     {
//         // check if the next block is allocated
//         if (!GET_ALLOC(next_header_ptr))
//         {
//             // get the size of the next block
//             size_t next_block_size = GET_SIZE(next_header_ptr);

//             // if the next block size is greater than the required size
//             if (next_block_size + block_size >= size)
//             {
//                 // remove the next block from the free list
//                 remove_free_block(next_header_ptr);
//                 // allocate the current block
//                 PUT(HDRP(header_ptr), PACK(block_size + next_block_size, 1));
//                 // set the footer of the current block
//                 PUT(FTRP(header_ptr), PACK(block_size + next_block_size, 1));
//                 // set the header of the next block
//                 PUT(HDRP(NEXT_BLKP(header_ptr)), PACK(0, 1));
//                 return ptr;
//             }
//             // if the next block size is not enough
//             else
//             {
//                 void *new_block = mm_malloc(size);
//                 if (new_block == NULL)
//                 {
//                     return NULL;
//                 }
//                 // copy the data from the original block to the new block
//                 memcpy(new_block, ptr, block_size);
//                 // free the original block
//                 mm_free(ptr);
//                 return new_block;
//             }
//         }
//         // if the next block is allocated
//         else
//         {
//             void *new_block = mm_malloc(size);
//             if (new_block == NULL)
//             {
//                 return NULL;
//             }
//             // copy the data from the original block to the new block
//             memcpy(new_block, ptr, block_size);
//             // free the original block
//             mm_free(ptr);
//             return new_block;
//         }
//     }
//     // if the block is larger than the size requested
//     else
//     {
//         // check if the next block is allocated
//         if (!GET_ALLOC(next_header_ptr))
//         {
//             // get the size of the next block
//             size_t next_block_size = GET_SIZE(next_header_ptr);

//             // if the next block size plus the current block size is greater than the required size
//             if (next_block_size + block_size >= size)
//             {
//                 // remove the next block from the free list
//                 remove_free_block(next_header_ptr);
//                 // allocate the current block
//                 PUT(HDRP(header_ptr), PACK(size, 1));
//                 // set the footer of the current block
//                 PUT(FTRP(header_ptr), PACK(size, 1));
//                 // set the header of the next block
//                 PUT(HDRP(NEXT_BLKP(header_ptr)), PACK(next_block_size + block_size - size, 0));
//                 // set the footer of the next block
//                 PUT(FTRP(NEXT_BLKP(header_ptr)), PACK(next_block_size + block_size - size, 0));
//                 // add the next block to the free list
//                 add_free_block(NEXT_BLKP(header_ptr));
//                 return ptr;
//             }
//             // if the next block size is not enough
//             else
//             {
//                 void *new_block = mm_malloc(size);
//                 if (new_block == NULL)
//                 {
//                     return NULL;
//                 }
//                 // copy the data from the original block to the new block
//                 memcpy(new_block, ptr, block_size);
//                 // free the original block
//                 mm_free(ptr);
//                 return new_block;
//             }
//         }
//         // if the next block is allocated
//         else
//         {
//             void *new_block = mm_malloc(size);
//             if (new_block == NULL)
//             {
//                 return NULL;
//             }
//             // copy the data from the original block to the new block
//             memcpy(new_block, ptr, block_size);
//             // free the original block
//             mm_free(ptr);
//             return new_block;
//         }
//     }
//     return NULL;
// }


// 여기는 디버깅을 위한 코드로 추측 
// /*
//  * checkblock - Check the block for consistency
//  */
// static void checkblock(void *bp)
// {
//     if ((size_t)bp % 8)
//         printf("Error: %p is not doubleword aligned\n", bp);
//     if (GET(HDRP(bp)) != GET(FTRP(bp)))
//         printf("Error: header does not match footer\n");
// }

// /*
//  * mm_checkheap - Check the heap for consistency
//  */
// static void mm_checkheap(int verbose)
// {
//     char *bp = heap_listp;

//     if (verbose)
//         printf("Heap (%p):\n", heap_listp);

//     if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
//         printf("Bad prologue header\n");
//     checkblock(heap_listp);

//     for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
//     {
//         if (verbose)
//             printblock(bp);
//         checkblock(bp);
//     }

//     if (verbose)
//         printblock(bp);
//     if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
//         printf("Bad epilogue header\n");
// }

// /*
//  * printblock - Print the block for debugging
//  */
// static void printblock(void *bp)
// {
//     size_t hsize, halloc, fsize, falloc;

//     mm_checkheap(0);
//     hsize = GET_SIZE(HDRP(bp));
//     halloc = GET_ALLOC(HDRP(bp));
//     fsize = GET_SIZE(FTRP(bp));
//     falloc = GET_ALLOC(FTRP(bp));

//     if (hsize == 0)
//     {
//         printf("%p: EOL\n", bp);
//         return;
//     }

//     printf("%p: header: [%d:%c] footer: [%d:%c]\n", bp,
//            hsize, (halloc ? 'a' : 'f'),
//            fsize, (falloc ? 'a' : 'f'));
// }
