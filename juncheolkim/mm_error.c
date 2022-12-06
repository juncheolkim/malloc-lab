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

/* explicit list constants and macros */
int NIL = 0;
char *root_free = &NIL;
#define NEXT_NEXT_FREE(bp) ((char *)NEXT_BLKP(bp) + WSIZE )
#define PREV_NEXT_FREE(bp) ((char *)PREV_BLKP(bp) + WSIZE )

static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (4*DSIZE)) // explicit
    {
        PUT(HDRP(bp), PACK(asize,1));
        PUT(FTRP(bp), PACK(asize,1));
        PUT(NEXT_BLKP(bp), GET(bp)); // explicit
        PUT(NEXT_NEXT_FREE(bp) , GET((char *)bp + WSIZE)); // explicit
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
    }
    else {
        if (root_free == (char *)bp) // explicit
        {
            root_free = (char *)GET(bp+WSIZE);
        }
        else // explicit
        { 
            PUT(PREV_NEXT_FREE(bp), GET((char *)bp + WSIZE));
        }
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}


static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc){ // case 1 
        return bp;
    }
    else if (prev_alloc && !next_alloc) { // case 2 
    
        PUT( (char *)(GET((char *)NEXT_BLKP(bp)) + WSIZE) , GET( (char *)((char *)NEXT_BLKP(bp)+WSIZE)) ); // explicit
        
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc) { // case 3 
        PUT( (char *)GET(PREV_BLKP(bp)) + WSIZE , GET(PREV_BLKP(bp)+WSIZE)); // explicit
        PUT(PREV_BLKP(bp), 0); // explicit
        PUT(PREV_BLKP(bp)+WSIZE, GET(bp+WSIZE)); // explicit
        
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else { // case 4
        PUT( GET(NEXT_BLKP(bp)) + WSIZE , GET(NEXT_BLKP(bp)+WSIZE)); // explicit
        PUT( GET(PREV_BLKP(bp)) + WSIZE , GET(PREV_BLKP(bp)+WSIZE)); // explicit
        PUT(PREV_BLKP(bp), 0); // explicit
        PUT(PREV_BLKP(bp)+WSIZE, GET(bp+WSIZE)); // explicit

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
    if ((long) (bp = mem_sbrk(size)) == -1)
        return NULL;
    PUT(HDRP(bp) , PACK(size, 0));
    PUT(FTRP(bp) , PACK(size, 0));
    if (!root_free) { // root_free 
        root_free = (int *)bp; // explicit
        PUT(bp, 0); // explicit
        PUT(bp+WSIZE, 0); // explicit
    }
    else {
        PUT(bp+WSIZE, *(char *)root_free); // explicit
        PUT(root_free, *(char *)bp); // explicit
        root_free = (int *)bp; // explicit
        PUT(root_free, 0); // explicit
    }
    
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
    void *bp;

    // implicit find_first_fit
    // for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    //     if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
    //         return bp; // find-fit
    //     }
    // }


    // explicit find_fist_fit
    for (bp = root_free; *(char *)bp != 0; bp = (char *)GET(bp + WSIZE)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
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

    if (size == 0)
        return NULL;
    
    if (size <= DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE );
    
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }
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
    PUT(ptr, 0); // explicit
    PUT((char *)(ptr + WSIZE) , *(char *)root_free); // explicit
    if (root_free != 0) // explicit
    {
        PUT(root_free, *(char *)ptr);
    }
    root_free = ptr; // explicit
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