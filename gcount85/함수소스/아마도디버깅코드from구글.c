
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
