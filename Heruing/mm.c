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
		"team2",
		/* First member's full name */
		"Eunmin Lee",
		/* First member's email address */
		"a@b",
		/* Second member's full name (leave blank if none) */
		"Juncheol KIM",
		/* Second member's email address (leave blank if none) */
		"a@b"};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE 4							/* Word and header/footer size (bytes) */
#define DSIZE 8							/* Double word size (bytes) */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) */
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

/* For segmregated free list*/
#define LISTLIMIT 20
#define SUCC_BLCK (*(void **)(bp))
#define PRED_BLCK (*(void **)(bp + WSIZE))

static char *heap_listp;
static char *findp;
// static void *segregation_list[LISTLIMIT];
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t adjusted_size);
static void place(void *bp, size_t adjusted_size);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
	int list;
	/* Create segregated free list*/
	// for (list = 0; list < LISTLIMIT; list++)
	// {
	// 	segregation_list[list] = NULL;
	// }

	/* Create the initial empty heap - if max memory is smaller than requested memory: return -1 */
	if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
		return -1;
	PUT(heap_listp, 0);														 /* Alignment padding */
	PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header */
	PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
	PUT(heap_listp + (3 * WSIZE), PACK(0, 1));		 /* Epilogue header */
	heap_listp += (2 * WSIZE);

	/* Extend the empty heap with a free block of CHUNKSIZE bytes */
	if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
		return -1;
	findp = (char *)heap_listp; // have to casting: heap_listp is void
	return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	char *bp;
	size_t asize;			 /* Adjusted block size */
	size_t extendsize; /* Amount to extend heap if no fit */
	/* Ignore spurious requests */
	if (size == 0)
		return NULL;

	/* Adjust block size to include overhead and alignment reqs. */
	if (size <= DSIZE)
		asize = 2 * DSIZE;
	else
		asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

	/* Search the free list for a fit */
	if ((bp = find_fit(asize)) != NULL)
	{
		place(bp, asize);
		findp = bp;
		return bp;
	}

	/* No fit found. Get more memory and place the block */
	extendsize = MAX(asize, CHUNKSIZE);
	if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
		return NULL;
	place(bp, asize);
	findp = bp;
	return bp;
}

/*
 * find_fit - memory allocation rule.
 */
static void *find_fit(size_t asize)
{
	/* First-fit search */
	// void *bp;
	// for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
	// {
	// 	if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
	// 	{
	// 		return bp;
	// 	}
	// }
	// return NULL; /* No fit */

	/* Next-fit search */
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

	/* Best-fit search*/
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
	size_t size = GET_SIZE(HDRP(bp));
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	coalesce(bp);
}

/*
 * extend_heap - extends the heap with a new free block
 */
static void *extend_heap(size_t words)
{
	char *bp;
	size_t size;
	/* Allocate an even number of words to maintain alignment */
	size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE; // double-word, if words%2 == 1: words++
	if ((long)(bp = mem_sbrk(size)) == -1)										// error
		return NULL;

	/* Initialize free block header/footer and the epilogue header */
	PUT(HDRP(bp), PACK(size, 0));					/* Free block header */
	PUT(FTRP(bp), PACK(size, 0));					/* Free block footer */
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

	/* Coalesce if the previous block was free */
	return coalesce(bp);
}

/*
 * coalesce - coalescing to merge it with any adjacent free blocks.
 */
static void *coalesce(void *bp)
{
	/* check prev & next blocks */
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));

	/* Case 1 - Allocated prev & allocated next blocks */
	if (prev_alloc && next_alloc)
	{
		findp = bp;
		return bp;
	}
	else if (prev_alloc && !next_alloc)
	{ /* Case 2 - Allocated prev and free next blocks*/
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}
	else if (!prev_alloc && next_alloc)
	{ /* Case 3 - Free prev and allocated next blocks*/
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(FTRP(bp), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);
	}
	else
	{ /* Case 4 - Free prev and free next blocks*/
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
						GET_SIZE(FTRP(NEXT_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);
	}
	findp = bp;
	return bp;
}

/*
 * place
 */
static void place(void *bp, size_t asize)
{
	size_t csize = GET_SIZE(HDRP(bp));
	if ((csize - asize) >= (2 * DSIZE))
	{
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		bp = NEXT_BLKP(bp);
		PUT(HDRP(bp), PACK(csize - asize, 0));
		PUT(FTRP(bp), PACK(csize - asize, 0));
	}
	else
	{
		PUT(HDRP(bp), PACK(csize, 1));
		PUT(FTRP(bp), PACK(csize, 1));
	}
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
	void *oldptr = ptr;
	void *newptr;
	size_t newSize = ALIGN(size) + DSIZE;
	size_t oldSize;
	if (size <= 0)
		return NULL;

	newptr = mm_malloc(newSize); //	new pointer to allocate
	if (newptr == NULL)
		return NULL;

	oldSize = GET_SIZE(HDRP(oldptr)) - DSIZE; //*(size_t *)((char *)oldptr - SIZE_T_SIZE);
	if (newSize < oldSize)
		oldSize = newSize;

	memcpy(newptr, oldptr, oldSize);
	mm_free(oldptr);
	return newptr;
}