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
    "2team",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE 4 /* Word a끄d header/footer size (bytes) */
#define DSIZE 8 /* Double word size (bytes) */
#define CHUNKSIZE (1<<12) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ( (char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(( (char *)(bp) - DSIZE)))

/* set root free block */
char *ROOT_FREE = NULL;


static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (2*DSIZE)) // 가용 블럭 최소값이 보장될 경우
    {
        PUT(HDRP(bp), PACK(asize,1));
        PUT(FTRP(bp), PACK(asize,1));

        PUT(NEXT_BLKP(bp), GET(bp)); // explicit
        // printf("%d\n",GET(bp));
        PUT((NEXT_BLKP(bp)+WSIZE), GET((char *)bp+WSIZE)); // explicit

        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));

    }
    else { // 가용 블럭 최소값이 보장될 수 없는 경우 > 블럭 전체를 먹는다
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        
        if (!GET(bp) && GET((char *)(bp) + WSIZE)) { //explicit (case1), root_free 블럭일 경우, next가 있을 경우
            ROOT_FREE = (char *)GET((char *)(bp) + WSIZE); // root_free 값은 기존 root_free값의 next 값이 된다
            PUT(ROOT_FREE,(unsigned int)NULL); // explicit
        }
        else if (GET(bp) && !GET((char *)(bp) + WSIZE)) { //explicit (case2), 마지막 블럭일 경우, prev가 있을 경우
            ROOT_FREE = (char *)GET(bp); 
            PUT(ROOT_FREE+WSIZE,(unsigned int)NULL);
        }
        else if (!GET(bp) && !GET((char *)(bp) + WSIZE)) { // explicit (case3), 루트 블럭이며, prev와 next가 없는 경우
            ROOT_FREE = NULL;
        }
        else { // explicit (case4), prev와 next가 존재하는 경우
            PUT( ((char *)GET(bp)+WSIZE) , GET((char *)bp + WSIZE)); // explicit, predecessor 수정
            PUT( GET((char *)bp + WSIZE) , GET(bp) ); // explicit, successor 수정
        }

    }
}


static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    printf("prev_alloc : %u, next_alloc : %u\n", prev_alloc, next_alloc);
    if (prev_alloc && next_alloc){ // case 1
        printf("case1\n");
        return bp;
    }
    else if (prev_alloc && !next_alloc) { // case 2 우측 가용 블럭과 병합
        printf("case2\n");
        if ( (unsigned int)GET(NEXT_BLKP(bp)+WSIZE) == 0 ) { // explicit, 우측 블럭이 마지막 블럭이었을 때
            PUT( NEXT_BLKP(bp)+WSIZE, GET(NEXT_BLKP(bp)+WSIZE) ); // explicit
        }
        else {
            PUT( NEXT_BLKP(bp)+WSIZE, GET(NEXT_BLKP(bp)+WSIZE) ); // explicit
            PUT( NEXT_BLKP(bp)+WSIZE , GET(NEXT_BLKP(bp)) ); // explicit
        }

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc) { // case 3 좌측 가용 블럭과 병합

        printf("case 3\n");
        if ( (unsigned int)(GET(PREV_BLKP(bp)+WSIZE)) == 0 ) { // explicit, 좌측 블럭이 마지막 블럭이었을 때
            printf("case 3, into case3_1\n");
            PUT( PREV_BLKP(bp)+WSIZE , (unsigned int)GET(PREV_BLKP(bp)+WSIZE) ); // explicit
        }
        else {
            PUT( PREV_BLKP(bp)+WSIZE, GET(PREV_BLKP(bp)+WSIZE) ); // explicit
            PUT( PREV_BLKP(bp)+WSIZE , GET(PREV_BLKP(bp)) ); // explicit
        }
        PUT( PREV_BLKP(bp), GET(bp) ); // explicit
        PUT(PREV_BLKP(bp)+WSIZE , GET((char *)bp + WSIZE)); // explicit

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else { // case 4 양측 가용 블럭과 병합
        printf("case 4\n");
        if ( (unsigned int)GET(NEXT_BLKP(bp)+WSIZE) == 0 ) { // explicit, 우측 블럭이 마지막 블럭이었을 때
            PUT( ((char *)GET(NEXT_BLKP(bp))+WSIZE), GET(NEXT_BLKP(bp)+WSIZE) ); // explicit
            PUT( (char *)GET(PREV_BLKP(bp))+WSIZE, GET(PREV_BLKP(bp)+WSIZE) ); // explicit
            PUT( (char *)GET(PREV_BLKP(bp)+WSIZE) , GET(PREV_BLKP(bp)) ); // explicit
        }
        else if (GET(PREV_BLKP(bp)+WSIZE) == 0) { // explicit, 좌측 블럭이 마지막 블럭이었을 때
            PUT( ((char *)GET(NEXT_BLKP(bp))+WSIZE), GET(NEXT_BLKP(bp)+WSIZE) ); // explicit
            PUT( ((char *)GET(NEXT_BLKP(bp)+WSIZE)) , GET(NEXT_BLKP(bp)) ); // explicit
            PUT( (char *)GET(PREV_BLKP(bp))+WSIZE, GET(PREV_BLKP(bp)+WSIZE) ); // explicit
        }
        else {
            PUT( ((char *)GET(NEXT_BLKP(bp))+WSIZE), GET(NEXT_BLKP(bp)+WSIZE) ); // explicit
            PUT( ((char *)GET(NEXT_BLKP(bp)+WSIZE)) , GET(NEXT_BLKP(bp)) ); // explicit
            PUT( (char *)GET(PREV_BLKP(bp))+WSIZE, GET(PREV_BLKP(bp)+WSIZE) ); // explicit
            PUT( (char *)GET(PREV_BLKP(bp)+WSIZE) , GET(PREV_BLKP(bp)) ); // explicit
        }
        PUT( PREV_BLKP(bp), GET(bp) ); // explicit
        PUT(PREV_BLKP(bp)+WSIZE , GET((char *)bp + WSIZE)); // explicit

        size += GET_SIZE(HDRP(PREV_BLKP(bp)))
            + GET_SIZE(FTRP(NEXT_BLKP(bp)));;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    printf("size : %d\n", size); // check
    if ((long) (bp = mem_sbrk(size)) == -1)
        return NULL;
    if (ROOT_FREE == NULL) { // explicit ,가용 블럭 X
        // printf("2. %p\n", ROOT_FREE); // check
        PUT(bp,(unsigned int)NULL);
        PUT(bp+WSIZE, (unsigned int)NULL);
        // printf("3. %d\n", *ROOT_FREE); // check
    }
    else { // explicit ,가용 블럭 O
        // printf("2. %p\n", ROOT_FREE); // check
        PUT(ROOT_FREE, (unsigned int)bp);
        PUT(bp, (unsigned int)NULL);
        PUT(bp + WSIZE, (unsigned int)ROOT_FREE);
        // printf("3. %p\n", (char *)(*(unsigned int*)(bp + WSIZE))); // check
    }
    ROOT_FREE = bp;
    PUT(HDRP(bp) , PACK(size, 0));
    PUT(FTRP(bp) , PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    

    return coalesce(bp);
}


/* 
 * mm_init - initialize the malloc package.
 */
char *heap_listp;

int mm_init(void)
{
    /* Create the initial empty heap */
    ROOT_FREE = NULL; // root 초기화
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1*WSIZE) , PACK(DSIZE, 1));
    PUT(heap_listp + (2*WSIZE) , PACK(DSIZE, 1));
    PUT(heap_listp + (3*WSIZE) , PACK(0, 1));
    heap_listp += (2*WSIZE) ;
    /* Extend the empty heap with a free block */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

static void *find_fit(size_t asize)
{
    // void *bp;

    // for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    //     if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
    //         return bp; // find-fit
    //     }
    // }

    void *bp;

    for (bp = ROOT_FREE; bp != NULL ; bp = (char *)GET((char *)bp + WSIZE)) {
        if ((asize <= GET_SIZE(HDRP(bp)))) {
            return bp; // find-fit
        }
    }

    return NULL; // No fit
}




/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;
    printf("malloc size : %u\n", size);

    if (size == 0)
        return NULL;
    
    if (size <= DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE );

    printf("malloc asize : %u\n", asize);
    if ((bp = find_fit(asize)) != NULL) {
        printf("heollo, it's me\n");
        place(bp, asize);
        return bp;
    }
    printf("find_fit_false\n");
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    if (ROOT_FREE) { // explicit, ROOT_FREE가 있었다면
        PUT(ROOT_FREE, (int)ptr); // explicit
    }
    PUT(ptr, (unsigned int)NULL); // explicit
    PUT((char *)ptr+WSIZE, (unsigned int)ROOT_FREE); // explicit
    ROOT_FREE = (char *)ptr; // explicit
    
    coalesce(ptr);
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
    copySize =  GET_SIZE(HDRP(oldptr)) - DSIZE;
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}