/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
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

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

/*
 * mm_init - initialize the malloc package.
 */

static void *extend_heap(size_t words);
int mm_init(void)
{
    static char *heap_listp; // 항상 힙의 프롤로그 블록을 가리킬 포인터 변수

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
    // 청크사이즈(4096)/워드사이즈(4) = 워드개수(1024)
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

// 책보고 추가
// 새 가용 블록으로 힙을 늘리기

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    
    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE; // 워드를 2로 나눠서 홀수면 워드 한개 더 추가
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    /* Initialize free block header/footer and the epilogue header */
    // 새로 받은 가용 블럭의 헤더/푸터와 에필로그 헤더를 초기화 
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp); // 통합된 블럭의 블럭 포인트 반환 
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
        return NULL;
    else
    {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
