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
    "🅱️alloc",
    /* First member's full name */
    "Michael Vu",
    /* First member's email address */
    "mike.vu@mail.utoronto.ca",
    /* Second member's full name 
    (do not modify this as this is an individual lab) */
    "",
    /* Second member's email address 
    (do not modify this as this is an individual lab)*/
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

// Heuristic, 1 for always split when possible.
// Currently, 2 seems to be a good heuristic.
#define SPLIT_SIZE_RATIO 2
#define MIN_SPLIT_SIZE SPLIT_SIZE_RATIO * MIN_BLOCK_SIZE

// Allocations under this size in bytes will use first fit
#define FIRST_FIT_THRESHOLD 63
#define CLOSE_FIT_THRESHOLD 32

#define FAST_LOG_2_FLOOR_BIT_OFFSET 5 
#define BINDEX_MAX_SIZE 13

static size_t SIZE_T_MAX = ~0;

void* free_lists[BINDEX_MAX_SIZE];

// #define STATS_ENABLED
#ifdef STATS_ENABLED
double stats_avg_size = 0;
long stats_avg_size_num = 0;
#endif

static int get_bindex(size_t asize)
{
    ASSERT(asize >= MIN_BLOCK_SIZE);

    if (asize == 32)
        return 0;

    if (asize < 40)
        return 1;

    if (asize < 48)
        return 2;

    if (asize < 56)
        return 3;

    if (asize < 64)
        return 4;

    if (asize < 128)
        return 5;

    if (asize < 256)
        return 6;

    if (asize < 512)
        return 7;

    if (asize < 1024)
        return 8;

    if (asize < 2048)
        return 9;

    if (asize < 4096)
        return 10;

    if (asize < 8192)
        return 11;

    return 12;
}

static inline void** get_free_list(size_t asize)
{
    ASSERT(asize >= MIN_BLOCK_SIZE);
    ASSERT(get_bindex(asize) < BINDEX_MAX_SIZE);
    ASSERT(get_bindex(asize) >= 0);
    return free_lists + get_bindex(asize);
}

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void)
{
    void* heap_listp;
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;

    PUT(heap_listp, 0);                         // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header
    heap_listp += DSIZE;

    for (int i = 0; i < BINDEX_MAX_SIZE; ++i)
        free_lists[i] = NULL;

    return 0;
}

static void remove_from_free_list(void* bp, void** free_list_head)
{
    ASSERT(bp);
    ASSERT(free_list_head);
    ASSERT(*free_list_head);
    void* prev = PREV_FREE_BLKP(bp);
    void* next = NEXT_FREE_BLKP(bp);

    ASSERT((prev != next) || (prev == NULL && next == NULL));

    if (prev)
    {
        ASSERT(bp != *free_list_head);
        PUT_P(HDR_NEXT_P(prev), next);
    } else 
    {
        ASSERT(bp == *free_list_head);
        *free_list_head = next;
    }

    if (next)
    {
        PUT_P(HDR_PREV_P(next), prev);
    }

    // bp's pointers do not change.
}

// Assumes that bp is not currently inside the list.
static void insert_to_list_head(void* bp, void** free_list_head)
{
    ASSERT(bp);
    ASSERT(free_list_head);
    void* old_head = *free_list_head;
    PUT_P(HDR_PREV_P(bp), 0x0);
    PUT_P(HDR_NEXT_P(bp), old_head);
    if (old_head)
    {
        PUT_P(HDR_PREV_P(old_head), bp);
    }

    *free_list_head = bp;
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

    void* a_bp = PREV_BLKP(bp); // Pointer to the previous block (in memory)
    void* c_bp = NEXT_BLKP(bp); // Pointer to the next block (in memory)

    size_t prev_alloc = GET_ALLOC(((char*)bp) - DSIZE);
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    ASSERT(GET_SIZE(FTRP(bp)) == size);

    if (prev_alloc && next_alloc)
    {
        // Both allocated
        DEBUG("both allocated\n");
        return bp;
    } else if (prev_alloc && !next_alloc)
    {
        DEBUG("Next is free\n");

        // Coalesce with next block

        size_t c_size = GET_SIZE(HDRP(c_bp));

        // Remove c from the free list
        remove_from_free_list(c_bp, get_free_list(c_size));

        // Merge these blocks.
        size_t new_size = size + c_size;
        PUT(HDRP(bp), PACK(new_size, 0));
        PUT(FTRP(bp), PACK(new_size, 0));

        void** old_free_list = get_free_list(size);
        void** new_free_list = get_free_list(new_size);
        if (old_free_list != new_free_list)
        {
            // bp must be moved to the new free list
            remove_from_free_list(bp, old_free_list);
            insert_to_list_head(bp, new_free_list);
        }

        return (bp);
    } else if (!prev_alloc && next_alloc)
    {
        DEBUG("prev is free\n");
        // Coalesce with prev block

        // Remove the current block from the free list
        remove_from_free_list(bp, get_free_list(size));
        size_t a_size = GET_SIZE(HDRP(a_bp));
        size_t new_size = size + a_size;
        PUT(FTRP(bp), PACK(new_size, 0));
        PUT(HDRP(a_bp), PACK(new_size, 0));

        void** old_free_list = get_free_list(a_size);
        void** new_free_list = get_free_list(new_size);
        if (old_free_list != new_free_list)
        {
            remove_from_free_list(a_bp, old_free_list);
            insert_to_list_head(a_bp, new_free_list);
        }

        return (PREV_BLKP(bp));
    } else
    {   
        DEBUG("Both are free\n");

        size_t c_size = GET_SIZE(HDRP(c_bp));

        // Remove c from the list
        remove_from_free_list(c_bp, get_free_list(c_size));

        // Remove b from the list
        remove_from_free_list(bp, get_free_list(size));

        size_t a_size = GET_SIZE(HDRP(a_bp));

        // a's pointers do not change.
        size_t new_size = size + a_size + c_size;
        PUT(HDRP(PREV_BLKP(bp)), PACK(new_size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(new_size,0));

        void** old_free_list = get_free_list(a_size);
        void** new_free_list = get_free_list(new_size);

        if (old_free_list != new_free_list)
        {
            remove_from_free_list(a_bp, old_free_list);
            insert_to_list_head(a_bp, new_free_list);
        }

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
    insert_to_list_head(bp, get_free_list(size));

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}


void* find_fit_first_fit(register size_t asize)
{
    DEBUG("Finding fit size: %d\n", (int) asize);

    void** free_list_min = get_free_list(asize);
    void** free_list_end = free_lists + BINDEX_MAX_SIZE;

    for (void** curr_free_list_head = free_list_min; 
        curr_free_list_head != free_list_end; 
        curr_free_list_head++)
    {
        for (register uintptr_t* bp = (uintptr_t*) *curr_free_list_head; 
            bp != NULL; 
            bp = (uintptr_t*) *bp)
        {
            // First fit
            if (asize <= GET_SIZE(HDRP(bp)))
            {
                return bp;
            }
        }
    }

    return NULL;
}

void* find_fit_close_fit(register size_t asize)
{
    for (register uintptr_t* bp = (uintptr_t*) *get_free_list(asize);
        bp != NULL; 
        bp = (uintptr_t*) *bp)
    {
        register size_t curr_block_size = GET_SIZE(HDRP(bp));
        // First fit
        if (asize <= curr_block_size && curr_block_size - asize <= CLOSE_FIT_THRESHOLD)
        {
            return bp;
        }
    }
    return find_fit_first_fit(asize);
}

#define BEST_FIT_LIMIT 10
void* find_fit_best_fit(register size_t size)
{
    uintptr_t** free_list_min = (uintptr_t**) get_free_list(size);
    #ifdef STATS_ENABLED
        stats_avg_size += (double) get_bindex(size);
        stats_avg_size_num++;
        printf("%lf\n", stats_avg_size / stats_avg_size_num);
    #endif

    uintptr_t** free_list_end = (uintptr_t**) free_lists + BINDEX_MAX_SIZE;

    for (uintptr_t** curr_free_list_head = free_list_min; 
        curr_free_list_head != free_list_end; 
        curr_free_list_head++)
    {
        register void* best_fit_ptr = NULL;
        register size_t best_fit_size = SIZE_T_MAX;
        ASSERT(best_fit_size > 0);

        register int i;
        for (register uintptr_t* bp = *curr_free_list_head, i = 0;
            bp != NULL && i < BEST_FIT_LIMIT;

            bp = (uintptr_t*) *bp,
            i++)
        {
            // First fit
            register size_t block_size = *(bp - 1);
            if (size == block_size)
                return bp;
            else if (size < block_size && block_size < best_fit_size)
            {
                best_fit_ptr = bp;
                best_fit_size = block_size;
            }
        }

        if (best_fit_ptr)
            return best_fit_ptr;
    }

    return NULL;
}

static inline void* find_fit(size_t size)
{
    return find_fit_best_fit(size);
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize)
{
    ASSERT(bp);
    ASSERT(asize >= MIN_BLOCK_SIZE);
    DEBUG("Placing size: %d at 0x%lx\n", (int) asize, (unsigned long) bp);
    /* Get the current block size */
    size_t bsize = GET_SIZE(HDRP(bp));

    ASSERT(asize <= bsize);

    size_t selected_block_size = bsize;

    if (bsize - asize >= MIN_SPLIT_SIZE)
    {
        // Split the block.
        // TODO: somewhere in this block breaks the memory structure.
        void* new_bp = bp + asize;
        size_t new_block_size = bsize - asize;
        selected_block_size = asize;

        DEBUG("Splitting block. %d->(%d/%d)\n", 
            (int) bsize,
            (int)selected_block_size, (int)new_block_size);

        ASSERT(!(new_block_size % 16));
        ASSERT(!(selected_block_size % 16));

        // Overwrite the old footer
        PUT_P(FTRP(bp), PACK(new_block_size, 0));

        // Add the new header
        PUT_P(HDRP(new_bp), PACK(new_block_size, 0));

        // Add a footer for the old block
        PUT_P(new_bp - DSIZE, PACK(selected_block_size, 0));

        // Overwrite the header for the old block
        PUT_P(HDRP(bp), PACK(selected_block_size, 0));

        // Add the split block to the free list
        insert_to_list_head(new_bp, get_free_list(new_block_size));

        ASSERT(GET_SIZE(HDRP(bp)) == selected_block_size);
        ASSERT(GET_SIZE(FTRP(bp)) == selected_block_size);
        ASSERT(GET_SIZE(HDRP(new_bp)) == new_block_size);
        ASSERT(GET_SIZE(FTRP(new_bp)) == new_block_size);
        ASSERT(!GET(HDR_PREV_P(new_bp)));
    }

    remove_from_free_list(bp, get_free_list(bsize));

    // Mark this block as allocated.
    PUT(HDRP(bp), PACK(selected_block_size, 1));
    PUT(FTRP(bp), PACK(selected_block_size, 1));

    ASSERT(GET_ALLOC(HDRP(bp)));
    ASSERT(GET_ALLOC(FTRP(bp)));

    ASSERT(GET_SIZE(HDRP(bp)) == GET_SIZE(FTRP(bp)));
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

    ASSERT(GET_SIZE(FTRP(bp)) == size);

    // Insert the free block into the free list.
    void* old_head = *get_free_list(size);
    *get_free_list(size) = bp;

    // Link the inserted block to the old head of the list.
    PUT_P(HDR_NEXT_P(bp), old_head);
    PUT_P(HDR_PREV_P(bp), 0x0);
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

static size_t user_size_to_block_size(size_t user_requested_size)
{
    ASSERT(user_requested_size > 0);
    /* Adjust block size to include overhead and alignment reqs. */
    if (user_requested_size <= MIN_BLOCK_SIZE - DSIZE)
        return MIN_BLOCK_SIZE;
    else
        return DSIZE * ((user_requested_size + (DSIZE) + (DSIZE-1))/ DSIZE);
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

    // Change to block size.
    asize = user_size_to_block_size(size);

    ASSERT(asize >= size + DSIZE);
    ASSERT((asize & 0x1) != 0x1);
    ASSERT((asize % 8) == 0);

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

void* mm_realloc_grow(void* ptr, size_t block_size)
{
    ASSERT(ptr);
    ASSERT(block_size >= MIN_BLOCK_SIZE);
    ASSERT(!(block_size % 16));

    size_t current_block_size = GET_SIZE(HDRP(ptr));

    // Check if it can coalesce with the adjacent block (if free).
    void* next_bp = NEXT_BLKP(ptr);
    if (!GET_ALLOC(HDRP(next_bp)))
    {
        
        size_t next_block_size = GET_SIZE(HDRP(next_bp));
        
        size_t combined_size = current_block_size + next_block_size;
        if (block_size <= combined_size)
        {
            DEBUG("Reallocing by coalescing with next block: %ld "
                "(%ld,%ld)->%ld\n",
                (unsigned long) block_size,
                (unsigned long) current_block_size,
                (unsigned long) next_block_size,
                (unsigned long) combined_size);

            // Coalesce the blocks
            PUT_P(HDRP(ptr), PACK(combined_size, 1));
            PUT_P(FTRP(ptr), PACK(combined_size, 1));

            remove_from_free_list(next_bp, get_free_list(next_block_size));

            // Check if the resulting block can be split
            if (combined_size - block_size >= MIN_SPLIT_SIZE)
            {
                size_t new_free_bp_size = combined_size - block_size;
                // Overwrite the old footer
                PUT_P(HDRP(ptr), PACK(block_size, 1));
                PUT_P(FTRP(ptr), PACK(block_size, 1));

                void* new_free_bp = FTRP(ptr) + DSIZE;

                // Add the new header
                PUT_P(HDRP(new_free_bp), PACK(new_free_bp_size, 0));
                PUT_P(FTRP(new_free_bp), PACK(new_free_bp_size, 0));

                // Add the split block to the free list
                insert_to_list_head(
                    new_free_bp, 
                    get_free_list(new_free_bp_size)
                );
            }

            return ptr;
        }
    }

    void *oldptr = ptr;
    void *newptr;

    newptr = mm_malloc(block_size);
    if (newptr == NULL)
        return NULL;

    /* Copy the old data. */
    memcpy(newptr, oldptr, current_block_size);
    mm_free(oldptr);
    return newptr;
}

void* mm_realloc_shrink(void* ptr, size_t block_size)
{
    ASSERT(ptr);
    ASSERT(block_size >= MIN_BLOCK_SIZE);
    ASSERT(!(block_size % 16));

    size_t old_block_size = GET_SIZE(HDRP(ptr));
    size_t new_free_block_size = old_block_size - block_size;

    DEBUG("SHRINK: %ld->(%ld,%ld)\n", (unsigned long) old_block_size, 
        (unsigned long) block_size, (unsigned long) new_free_block_size);

    if (new_free_block_size < MIN_SPLIT_SIZE)
        return ptr; // Cannot create valid free space, no-op.

    // Shrink the headers of this block.
    PUT_P(HDRP(ptr), PACK(block_size, 1));
    PUT_P(FTRP(ptr), PACK(block_size, 1));

    void* new_bp = FTRP(ptr) + DSIZE;

    PUT_P(HDRP(new_bp), PACK(new_free_block_size, 0));
    PUT_P(FTRP(new_bp), PACK(new_free_block_size, 0));

    insert_to_list_head(new_bp, get_free_list(new_free_block_size));

    coalesce(new_bp);

    // Free the remaining space
    return ptr;
}   

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size)
{
    DEBUG("[[Realloc]] %lx to size %d\n", (unsigned long) ptr, (int) size);

    /* If size == 0 then this is just free, and we return NULL. */
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
        return (mm_malloc(size));

    size_t requested_block_size = user_size_to_block_size(size);
    size_t old_size = GET_SIZE(HDRP(ptr));

    if (requested_block_size == old_size)
        return ptr; // No size change.
    else if (requested_block_size > old_size)
        return mm_realloc_grow(ptr, requested_block_size);
    else
        return mm_realloc_shrink(ptr, requested_block_size);
}
