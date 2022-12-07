

// 데이터 보전하지 못했다는 에러 뜸 
// remove the next block from the free list
static void *remove_free_block(void *header_ptr)
{
    PUT(HDRP(header_ptr), PACK(GET_SIZE(HDRP(header_ptr)), 1));
    PUT(FTRP(header_ptr), PACK(GET_SIZE(HDRP(header_ptr)), 1));
}

// add the next block to the free list
static void *add_free_block(void *header_ptr)
{
    PUT(HDRP(header_ptr), PACK(GET_SIZE(HDRP(header_ptr)), 0));
    PUT(FTRP(header_ptr), PACK(GET_SIZE(HDRP(header_ptr)), 0));
}

void *mm_realloc(void *ptr, size_t size)
{
    size = ALIGN(size + DSIZE);

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
    size = ALIGN(size + DSIZE);

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




// 여기가 원본

// 복잡한 realloc 함수 -> 분석 필요
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