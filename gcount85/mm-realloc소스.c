
/*
 * @param ptr: pointer to previously malloc'ed memory
 * @param size: new size for memory
 *
 * @return returns a pointer to newly realloc'ed memory
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

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
// 여기 도통 이해가 안되는 부분임 ************************
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* 블록을 가리키는 포인터 bp가 주어졌을 때, 그것의 헤더와 풋터의 주소를 계산 */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* 블록을 가리키는 포인터 bp가 주어졌을 때, 다음 블럭과 이전 블럭의 주소를 계산 */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

void *mm_realloc(void *ptr, size_t size)
{
    // if ptr is NULL, then it is equivalent to malloc
    if (ptr == NULL)
    {
        return mm_malloc(size);
    }

    // if size is 0, then free the memory previously allocated
    if (size == 0)
    {
        mm_free(ptr);
        return NULL;
    }

    // get the header of the block
    void *header_ptr = HDRP(ptr);
    void *next_header_ptr = NEXT_BLKP(header_ptr);

    // get the size of the current block
    size_t block_size = GET_SIZE(HDRP(header_ptr));

    // if the block fits perfectly, no need to reallocate
    if (block_size == size)
    {
        return ptr;
    }
    // if the block is smaller than the size requested
    else if (block_size < size)
    {
        // check if the next block is allocated
        if (!GET_ALLOC(next_header_ptr))
        {
            // get the size of the next block
            size_t next_block_size = GET_SIZE(next_header_ptr);

            // if the next block size is greater than the required size
            if (next_block_size + block_size >= size)
            {
                // remove the next block from the free list
                remove_free_block(next_header_ptr);
                // allocate the current block
                PUT(HDRP(header_ptr), PACK(block_size + next_block_size, 1));
                // set the footer of the current block
                PUT(FTRP(header_ptr), PACK(block_size + next_block_size, 1));
                // set the header of the next block
                PUT(HDRP(NEXT_BLKP(header_ptr)), PACK(0, 1));
                return ptr;
            }
            // if the next block size is not enough
            else
            {
                void *new_block = mm_malloc(size);
                if (new_block == NULL)
                {
                    return NULL;
                }
                // copy the data from the original block to the new block
                memcpy(new_block, ptr, block_size);
                // free the original block
                mm_free(ptr);
                return new_block;
            }
        }
        // if the next block is allocated
        else
        {
            void *new_block = mm_malloc(size);
            if (new_block == NULL)
            {
                return NULL;
            }
            // copy the data from the original block to the new block
            memcpy(new_block, ptr, block_size);
            // free the original block
            mm_free(ptr);
            return new_block;
        }
    }
    // if the block is larger than the size requested
    else
    {
        // check if the next block is allocated
        if (!GET_ALLOC(next_header_ptr))
        {
            // get the size of the next block
            size_t next_block_size = GET_SIZE(next_header_ptr);

            // if the next block size plus the current block size is greater than the required size
            if (next_block_size + block_size >= size)
            {
                // remove the next block from the free list
                remove_free_block(next_header_ptr);
                // allocate the current block
                PUT(HDRP(header_ptr), PACK(size, 1));
                // set the footer of the current block
                PUT(FTRP(header_ptr), PACK(size, 1));
                // set the header of the next block
                PUT(HDRP(NEXT_BLKP(header_ptr)), PACK(next_block_size + block_size - size, 0));
                // set the footer of the next block
                PUT(FTRP(NEXT_BLKP(header_ptr)), PACK(next_block_size + block_size - size, 0));
                // add the next block to the free list
                add_free_block(NEXT_BLKP(header_ptr));
                return ptr;
            }
            // if the next block size is not enough
            else
            {
                void *new_block = mm_malloc(size);
                if (new_block == NULL)
                {
                    return NULL;
                }
                // copy the data from the original block to the new block
                memcpy(new_block, ptr, block_size);
                // free the original block
                mm_free(ptr);
                return new_block;
            }
        }
        // if the next block is allocated
        else
        {
            void *new_block = mm_malloc(size);
            if (new_block == NULL)
            {
                return NULL;
            }
            // copy the data from the original block to the new block
            memcpy(new_block, ptr, block_size);
            // free the original block
            mm_free(ptr);
            return new_block;
        }
    }
    return NULL;
}

/**
 * add_free_block function
 *
 * @param ptr: pointer to the block to be added
 *
 * Adds a block to the beginning of the free list
 */
static void add_free_block(void *ptr)
{
    // set the previous of the current block to point to the beginning of the free list
    PUT(PREV_LINKNODE_PTR(ptr), (int)free_listp);
    // set the next of the current block to point to the next of the beginning of the free list
    PUT(NEXT_LINKNODE_PTR(ptr), (int)NEXT_FREE_BLKP(free_listp));
    // set the previous of the next of the beginning of the free list to point to the current block
    PUT(PREV_LINKNODE_PTR(NEXT_FREE_BLKP(free_listp)), (int)ptr);
    // set the next of the beginning of the free list to point to the current block
    PUT(NEXT_LINKNODE_PTR(free_listp), (int)ptr);
}

/**
 * remove_free_block function
 *
 * @param ptr: pointer to the block to be removed
 *
 * Removes a block from the free list
 */
static void remove_free_block(void *ptr)
{
    // set the previous of the next of the current block to point to the previous of the current block
    PUT(PREV_LINKNODE_PTR(NEXT_FREE_BLKP(ptr)), (int)PREV_FREE_BLKP(ptr));
    // set the next of the previous of the current block to point to the next of the current block
    PUT(NEXT_LINKNODE_PTR(PREV_FREE_BLKP(ptr)), (int)NEXT_FREE_BLKP(ptr));
}

/**
 * add_free_block function
 *
 * @param ptr: pointer to the block to be added
 *
 * Adds a block to the beginning of the free list
 */
static void add_free_block(void *ptr)
{
    // set the previous of the current block to point to the beginning of the free list
    PUT(PREV_LINKNODE_PTR(ptr), (int)free_listp);
    // set the next of the current block to point to the next of the beginning of the free list
    PUT(NEXT_LINKNODE_PTR(ptr), (int)NEXT_FREE_BLKP(free_listp));
    // set the previous of the next of the beginning of the free list to point to the current block
    PUT(PREV_LINKNODE_PTR(NEXT_FREE_BLKP(free_listp)), (int)ptr);
    // set the next of the beginning of the free list to point to the current block
    PUT(NEXT_LINKNODE_PTR(free_listp), (int)ptr);
}

/**
 * remove_free_block function
 *
 * @param ptr: pointer to the block to be removed
 *
 * Removes a block from the free list
 */
static void remove_free_block(void *ptr)
{
    // set the previous of the next of the current block to point to the previous of the current block
    PUT(PREV_LINKNODE_PTR(NEXT_FREE_BLKP(ptr)), (int)PREV_FREE_BLKP(ptr));
    // set the next of the previous of the current block to point to the next of the current block
    PUT(NEXT_LINKNODE_PTR(PREV_FREE_BLKP(ptr)), (int)NEXT_FREE_BLKP(ptr));
}