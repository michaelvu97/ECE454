
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "defs.h"
#include "hash_element_lock.h"

#define SAMPLES_TO_COLLECT   10000000
#define RAND_NUM_UPPER_BOUND   100000
#define NUM_SEED_STREAMS            4

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "🅱️Thread",                  /* Team name */

    "Michael Vu",                    /* Member full name */
    "1002473272",                 /* Member student number */
    "mike.vu@mail.utoronto.ca",                 /* Member email address */
};

unsigned num_threads;
unsigned samples_to_skip;

class sample;

class sample {
    unsigned my_key;
    unsigned count;
public:
    pthread_mutex_t lock;
    sample *next;
    
    void increment();

    unsigned get_count();

    sample(unsigned the_key){my_key = the_key; count = 0;};
    unsigned key(){return my_key;}
    void print(FILE *f){printf("%d %d\n",my_key,count);}
};

// Not thread-safe
unsigned sample::get_count()
{
    return count;
}

// Thread-safe
void sample::increment()
{
    pthread_mutex_lock(&lock);
    count++;
    pthread_mutex_unlock(&lock);
}

// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned". 
// A thread-safe hash table, using list-level locks.
hash<sample,unsigned> h;

struct worker_args {
    int num_seed_streams;
    int seed_start;
};

void* worker_thread(void* args_ptr)
{
    struct worker_args worker_args = *((struct worker_args*) args_ptr);

    // process streams starting with different initial numbers
    for (
        int i = worker_args.seed_start; 
        i < worker_args.seed_start + worker_args.num_seed_streams;
        i++)
    {
        int rnum = i;

        // collect a number of samples
        for (int j = 0; j < SAMPLES_TO_COLLECT; j++)
        {
            // skip a number of samples
            for (int k = 0; k < samples_to_skip; k++)
            {
                rnum = rand_r((unsigned int*)&rnum);
            }

            // force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
            unsigned key = rnum % RAND_NUM_UPPER_BOUND;
            h.lookup_or_create(key)->increment();
        }
    }
}

int main (int argc, char* argv[])
{

    // Print out team information
    printf( "Team Name: %s\n", team.team );
    printf( "\n" );
    printf( "Student 1 Name: %s\n", team.name1 );
    printf( "Student 1 Student Number: %s\n", team.number1 );
    printf( "Student 1 Email: %s\n", team.email1 );
    printf( "\n" );

    // Parse program arguments
    if (argc != 3)
    {
        printf("Usage: %s <num_threads> <samples_to_skip>\n", argv[0]);
        exit(1);  
    }
    sscanf(argv[1], " %d", &num_threads); // not used in this single-threaded version
    sscanf(argv[2], " %d", &samples_to_skip);

    // initialize a 16K-entry (2**14) hash of empty lists
    h.setup(14);

    // Create worker threads
    struct worker_args arg[4];
    pthread_t threads[4];
    switch(num_threads)
    {
        case 1:
            arg[0].num_seed_streams = 4;
            arg[0].seed_start = 0;
            break;
        case 2:
            arg[0].num_seed_streams = 2;
            arg[0].seed_start = 0;
            arg[1].num_seed_streams = 2;
            arg[1].seed_start = 2;
            break;
        case 4:
            for (int i = 0; i < 4; i++)
            {
                arg[i].num_seed_streams = 1;
                arg[i].seed_start = i;
            }
            break;
    }

    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, &worker_thread, &arg[i]);

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    // print a list of the frequency of all samples
    h.print();
}
