/*
 * This implementation replicates the implicit list implementation
 * provided in the textbook
 * "Computer Systems - A Programmer's Perspective"
 * Blocks are never coalesced or reused.
 * Realloc is implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Jackson",
    /* First member's full name */
    "Michael Vu",
    /* First member's email address */
    "mike.vu@mail.utoronto.ca",
    /* Second member's full name (do not modify this as this is an individual lab) */
    "",
    /* Second member's email address (do not modify this as this is an individual lab)*/
    ""
};

// Comment this out to disable debug statements
// #define DEBUGGING_ENABLED
#ifdef DEBUGGING_ENABLED
    #define DEBUG(...) printf(__VA_ARGS__)
#else
    #define DEBUG(...)
#endif

// Comment this out to disable assertions
#define ASSERTIONS_ENABLED
#ifdef ASSERTIONS_ENABLED
    #define ASSERT(x) if (!(x)) \
    { \
        printf("Assertion failed: %s, %s:%d\n", #x, __FUNCTION__, __LINE__); \
        abort(); \
    }
#else
    #define ASSERT(x)
#endif

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
*************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)      /* initial heap size (bytes) */

#define MAX(x,y) ((x) > (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))
#define PUT_P(p,val)    (*(uintptr_t *)(p) = ((uintptr_t) val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define HDR_NEXT_P(bp) ((char*) (bp))
#define HDR_PREV_P(bp) (((char*) (bp)) + WSIZE)

#define NEXT_FREE_BLKP(bp) *((void**)HDR_NEXT_P(bp))
#define PREV_FREE_BLKP(bp) *((void**)HDR_PREV_P(bp))

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define MIN_BLOCK_SIZE 4 * WSIZE

void* heap_listp = NULL;

void* free_list_head = NULL;

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;

    PUT(heap_listp, 0);                         // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header
    heap_listp += DSIZE;

    free_list_head = NULL;

    return 0;
}

static void remove_from_free_list(void* bp)
{
    ASSERT(bp);
    void* prev = PREV_FREE_BLKP(bp);
    void* next = NEXT_FREE_BLKP(bp);

    ASSERT((prev != next) || (prev == NULL && next == NULL));

    if (prev)
    {
        ASSERT(bp != free_list_head);
        PUT_P(HDR_NEXT_P(prev), next);
    } else 
    {
        ASSERT(bp == free_list_head);
        free_list_head = next;
    }

    if (next)
    {
        PUT_P(HDR_PREV_P(next), prev);
    }

    // bp's pointers do not change.
}

// Assumes that bp is not currently inside the list.
static void insert_to_list_head(void* bp)
{
    ASSERT(bp);
    PUT_P(HDR_PREV_P(bp), 0x0);
    PUT_P(HDR_NEXT_P(bp), free_list_head);
    if (free_list_head)
    {
        PUT_P(HDR_PREV_P(free_list_head), bp);
    }
}


/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 * bp must be a free block which is currently inside the free list.
 *
 **********************************************************/
void *coalesce(void *bp)
{
    ASSERT(!GET_ALLOC(HDRP(bp)));

    DEBUG("Coalescing %lx: ", (unsigned long) bp);

    size_t prev_alloc = GET_ALLOC(((char*)bp) - 2 * WSIZE);
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)
    {
        // Both allocated
        DEBUG("both allocated\n");
        return bp;
    } else if (prev_alloc && !next_alloc)
    {
        DEBUG("Next is free\n");

        // Coalesce with next block
        void* c_bp = NEXT_BLKP(bp);

        // Remove c from the free list
        remove_from_free_list(c_bp);

        // Merge these blocks.
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        return (bp);
    } else if (!prev_alloc && next_alloc)
    {
        DEBUG("prev is free\n");
        // Coalesce with prev block

        // Remove the current block from the free list
        remove_from_free_list(bp);

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        return (PREV_BLKP(bp));
    } else
    {   
        DEBUG("Both are free\n");

        // c is bp's right child (next of bp in memory)
        void* c_bp = NEXT_BLKP(bp);

        // Remove c from the list
        remove_from_free_list(c_bp);

        // Remove b from the lsit
        remove_from_free_list(bp);

        // a's pointers do not change.

        size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));

        return (PREV_BLKP(bp));
    }
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void* extend_heap(size_t words)
{
    DEBUG("Extending Heap by %d words\n", (int)words);

    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ( (bp = mem_sbrk(size)) == (void *)-1 )
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    // This overwrites the previous epilogue
    ASSERT(GET_ALLOC(HDRP(bp)));
    ASSERT(GET_SIZE(HDRP(bp)) == 0);
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer

    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    // Insert the new free block into the linked list
    void* old_head = free_list_head;
    free_list_head = bp;
    PUT(HDR_PREV_P(bp), (uintptr_t) 0x0);
    PUT(HDR_NEXT_P(bp), (uintptr_t) old_head);
    if (old_head != NULL)
    {
        // Backwards link
        PUT(HDR_PREV_P(old_head), (uintptr_t) bp);
    }

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}


/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize)
{
    DEBUG("Finding fit size: %d\n", (int) asize);
    void *bp;
    for (bp = free_list_head; bp != NULL; bp = NEXT_FREE_BLKP(bp))
    {
        if (asize <= GET_SIZE(HDRP(bp)))
        {
            return bp;
        }
    }
    return NULL;
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize)
{
    DEBUG("Placing size: %d at 0x%lx\n", (int) asize, (unsigned long) bp);
    /* Get the current block size */
    size_t bsize = GET_SIZE(HDRP(bp));

    if (bsize - asize > MIN_BLOCK_SIZE)
    {
        // The block may be split.
    }

    // TODO: decide to split the block?
    // For now: the blocks aren't split.
    void* prev_bp = PREV_FREE_BLKP(bp);
    void* next_bp = NEXT_FREE_BLKP(bp);
    
    if (prev_bp == NULL)
    {
        // Case 1: The selected block is the head of the free list.       

        ASSERT(bp == free_list_head);

        // Reassign the root
        free_list_head = next_bp;

        if (next_bp)
        {
            // Reassign the previous ptr of the next element
            PUT(HDR_PREV_P(next_bp), 0x0);
        }
    } else 
    {
        ASSERT(bp != free_list_head);

        // Case 2: The selected block is inside or at the end of the free list.
        if (next_bp)
        {
            // Case 2.1: The selected block is not at the end of the free list.
            // Reassign the previous ptr of the next element
            PUT(HDR_PREV_P(next_bp), (uintptr_t) prev_bp);
        }

        // Reassign the next ptr of the previous element
        PUT(HDR_NEXT_P(prev_bp), (uintptr_t) next_bp);
    }

    // Mark this block as allocated.
    PUT(HDRP(bp), PACK(bsize, 1));
    PUT(FTRP(bp), PACK(bsize, 1));
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp)
{
    DEBUG("[[Freeing]] %lx\n", (unsigned long) bp);
    if (bp == NULL){
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));

    // Insert the free block into the free list.
    void* old_head = free_list_head;
    free_list_head = bp;

    // Link the inserted block to the old head of the list.
    PUT(HDR_NEXT_P(bp), (uintptr_t) old_head);
    PUT(HDR_PREV_P(bp), (uintptr_t) 0x0);
    if (old_head != NULL)
    {
        // The list was not empty before, add the backwards link to the
        // freed block
        PUT(HDR_PREV_P(old_head), (uintptr_t) bp);
    }

    // Mark the block as free
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));

    coalesce(bp);
}


/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size)
{
    DEBUG("[[Malloc]] %d\n", (int) size);
    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char * bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= MIN_BLOCK_SIZE)
        asize = MIN_BLOCK_SIZE;
    else
        // TODO: correct this alignment?
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;

}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size)
{
    DEBUG("[[Realloc]] %lx to size %d\n", (unsigned long) ptr, (int) size);
    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0){
      mm_free(ptr);
      return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
      return (mm_malloc(size));

    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;

    /* Copy the old data. */
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void){
  return 1;
}
