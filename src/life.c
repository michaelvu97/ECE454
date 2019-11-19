/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "pthread.h"
#include "life.h"
#include "util.h"
#include "stdlib.h"
#include "stdio.h"

#define NUM_WORKERS 4

#define FOREACH(i, lim) for (unsigned i = 0; i < lim; ++i)

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


/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/

typedef struct worker_args {
    int dim;
    int row_start;
    int row_end;
    int num_generations;
    char* source_board;
    char* target_board;
} worker_args;

int last_synced_generation = 0;
int num_done = 0;
pthread_mutex_t num_done_lock;
pthread_cond_t cv;
#define LOCK(x) pthread_mutex_lock(&x)
#define UNLOCK(x) pthread_mutex_unlock(&x)

void* worker(void* argsp)
{
    /*
      w  t a   sy wor     e
       ha    bu      ker b e
              __         .' '.
        _/__)        .   .       .
       (8|)_}}- .      .        .
        `\__)    '. . ' ' .  . '
    */
    worker_args args = *((worker_args*) argsp);
    int generation = 0;
    const int dim = args.dim;
    const int LAMBDA = dim - 1;
    char* source_board = args.source_board;
    char* target_board = args.target_board;


    while (generation < args.num_generations)
    {
        /*
         * DO WORK
         */

        // Their impl
        int lim = args.row_end;
        for (int row = args.row_start; row < lim; row++)
        {
            const int row_north = mod (row - 1, dim);
            const int row_south = mod (row + 1, dim);

            for (int col = 0; col < dim; col++)
            {
                const int col_west = mod (col - 1, dim);
                const int col_east = mod (col + 1, dim);

                const char neighbor_count = 
                    source_board[row_north * dim + col_west] + 
                    source_board[row_north * dim + col] + 
                    source_board[row_north * dim + col_east] + 
                    source_board[row * dim + col_west] +
                    source_board[row * dim + col_east] + 
                    source_board[row_south * dim + col_west] +
                    source_board[row_south * dim + col] + 
                    source_board[row_south * dim + col_east];

                target_board[col + dim*row] = alivep(neighbor_count, source_board[col + dim*row]);
            }
        }

        /*
        int lim = args.row_end;
        for (int row = args.row_start; row < lim; ++row)
        {
            const int inorth = mod(row - 1, dim);
            const int isouth = mod(row + 1, dim);

            // Check start col
            int neighbour_count = 
                source_board[(inorth * dim) + LAMBDA] +
                source_board[(inorth * dim)] +
                source_board[(inorth * dim) + 1] +
                source_board[(row * dim) + LAMBDA] + 
                source_board[(row * dim) + 1] + 
                source_board[(isouth * dim) + LAMBDA] + 
                source_board[(isouth * dim)] + 
                source_board[(isouth * dim) + 1];

            target_board[row * dim] = alivep(neighbour_count, source_board[row * dim]);

            // Check middle cols
            for (int col = 1; col < LAMBDA; ++col)
            {
                neighbour_count = 
                    source_board[(inorth * dim) + col - 1] +
                    source_board[(inorth * dim) + col] +
                    source_board[(inorth * dim) + col + 1] +
                    source_board[(row * dim) + col - 1] +
                    source_board[(row * dim) + col] +
                    source_board[(row * dim) + col + 1] +
                    source_board[(isouth * dim) + col - 1] +
                    source_board[(isouth * dim) + col] +
                    source_board[(isouth * dim) + col + 1];

                target_board[row * dim + col] = alivep(neighbour_count, source_board[row * dim]);
            }

            // Check end cols
            neighbour_count = 
                source_board[(inorth * dim) + LAMBDA - 1] +
                source_board[(inorth * dim) + LAMBDA] +
                source_board[(inorth * dim)] +
                source_board[(row * dim) + LAMBDA - 1] + 
                source_board[(row * dim)] + 
                source_board[(isouth * dim) + LAMBDA - 1] + 
                source_board[(isouth * dim) + LAMBDA] + 
                source_board[(isouth * dim)];

            target_board[row * dim + LAMBDA] = alivep(neighbour_count, source_board[row * dim]);
        }
        */

        /*
         * SYNCHRONIZE
         */
        LOCK(num_done_lock);
            ++num_done;
            if (num_done == NUM_WORKERS)
            {
                // Signal threads to wake up.
                pthread_cond_broadcast(&cv);
                num_done = 0;
                ++last_synced_generation;
            } else 
            {
                pthread_cond_wait(&cv, &num_done_lock);
            }
        UNLOCK(num_done_lock);

        char* temp = target_board;
        target_board = source_board;
        source_board = temp;

        generation++;
    }

    free(argsp);
    return NULL;
}

/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
    pthread_cond_init(&cv, NULL);
    pthread_t threads[NUM_WORKERS];    
    FOREACH(i, NUM_WORKERS)
    {
        worker_args* argsp = (worker_args*) malloc(sizeof(worker_args));
        argsp->num_generations = gens_max;
        argsp->dim = nrows;
        argsp->source_board = inboard;
        argsp->target_board = outboard;
        ASSERT(nrows / NUM_WORKERS);
        argsp->row_start = i * (nrows / NUM_WORKERS);

        if (i == NUM_WORKERS - 1)
            argsp->row_end = nrows;
        else
            argsp->row_end = (i + 1) * (nrows / NUM_WORKERS);

        ASSERT(argsp->row_start < argsp->row_end);

        pthread_create(threads + i, NULL, &worker, argsp);
    }

    FOREACH(i, NUM_WORKERS)
    {
        pthread_join(threads[i], NULL);
    }

    // TODO
    return gens_max % 2 ? outboard : inboard;
}
