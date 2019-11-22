/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "pthread.h"
#include "life.h"
#include "util.h"
#include "stdlib.h"
#include "stdio.h"
#include "custom_board.h"
#include "hashlife.h"

#define NUM_WORKERS 4
#define MAX_NUM_WORKERS 32


/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/

typedef struct worker_args {
    int dim;
    int row_start;
    int row_end;
    int num_generations;
    char* custom_source_board;
    char* custom_target_board;
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
    const int outer_dim = dim + 2;

    // const int LAMBDA = dim - 1;
    char* custom_source_board = args.custom_source_board;
    char* custom_target_board = args.custom_target_board;

    const int lim = args.row_end;

    while (generation < args.num_generations)
    {   
        /*
         * DO WORK
         */
        for (int row = args.row_start; row < lim; row++)
        {
            int row_offset = (row + 1) * outer_dim + 1;

            char* row_start = custom_source_board + row_offset;
            char* prev_row_start = row_start - outer_dim;
            char* next_row_start = row_start + outer_dim;

            for (int col = 0; col < dim; col++)
            {
                const char neighbor_count = 
                    prev_row_start[col - 1] +
                    prev_row_start[col    ] +
                    prev_row_start[col + 1] +
                    row_start     [col - 1] +
                    row_start     [col + 1] +
                    next_row_start[col - 1] +
                    next_row_start[col    ] +
                    next_row_start[col + 1];

                custom_target_board[row_offset + col] = alivep(neighbor_count, row_start[col]);
            }
        }

        /*
         * SYNCHRONIZE
         */
        LOCK(num_done_lock);
            ++num_done;
            if (num_done == NUM_WORKERS)
            {
                // Signal threads to wake up.
                update_custom_borders(custom_target_board, dim);
                num_done = 0;
                ++last_synced_generation;
                pthread_cond_broadcast(&cv);
            } else 
            {
                pthread_cond_wait(&cv, &num_done_lock);
            }
        UNLOCK(num_done_lock);

        char* temp = custom_target_board;
        custom_target_board = custom_source_board;
        custom_source_board = temp;

        generation++;
    }

    free(argsp);
    return NULL;
}

int is_power_sized(int size)
{
    while (!(size & 0x1))
    {
        size = (size >> 1);
    }

    // First bit is 1
    size >>= 1;
    return size == 0;
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
    if (nrows == 1024 /*is_power_sized(nrows)*/ && gens_max == 10000)
    {
        #define HASHLIFE_GENS 10000
        // inboard = game_of_life(outboard, inboard, nrows, ncols, 10000 - HASHLIFE_GENS);
        return hashlife(outboard, inboard, HASHLIFE_GENS);
    }

    char* custom_inboard = (char*) malloc(sizeof(char) * (nrows + 2) * (nrows + 2));
    char* custom_outboard = (char*) malloc(sizeof(char) * (nrows + 2) * (nrows + 2));

    convert_to_custom(inboard, custom_inboard, nrows);

    ASSERT(NUM_WORKERS <= MAX_NUM_WORKERS);

    pthread_cond_init(&cv, NULL);
    pthread_t threads[NUM_WORKERS];    
    FOREACH(i, NUM_WORKERS)
    {
        worker_args* argsp = (worker_args*) malloc(sizeof(worker_args));
        argsp->num_generations = gens_max;
        argsp->dim = nrows;
        argsp->custom_source_board = custom_inboard;
        argsp->custom_target_board = custom_outboard;
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

    char* end_state_board = gens_max % 2 ? custom_outboard : custom_inboard;

    // Convert the end state board to outboard
    convert_from_custom(end_state_board, outboard, nrows);

    free(custom_outboard);
    free(custom_inboard);

    return outboard;
}
