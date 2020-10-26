
// Common requirements of requester, resolver
#include "multi-lookup.h"
// Requirements of just multi-lookup
#include <pthread.h>
#include <sys/time.h>
#include "requester.h"
#include "resolver.h"

int main(int argc, char *argv[])
{
    struct timeval t1;
    struct timeval t2;
    float sec_elapsed;
    gettimeofday(&t1, NULL);

    puts("in main");

    // create & serialize pthread_create args
    int i;
    pthread_t tid1, tid2;
    struct ThreadArgs *thread_args = malloc(sizeof(struct ThreadArgs));

    thread_args->file_arr = make_mt_cirque();
    thread_args->shared_buff = make_mt_cirque();

    // TODO: this needs its own section / func
    printf("# requesters for this run: %s\n", argv[1]);
    printf("# resolvers for this run: %s\n", argv[2]);
    thread_args->log_path = argv[3];
    for (i = 5; i < argc; i++)
    {
        printf("queuing %s\n", argv[i]);
        mt_cirque_push(thread_args->file_arr, argv[i]);
    }

    // create a thread for parsing
    if (pthread_create(
            &tid1, NULL,
            requester_thread_func,
            &thread_args) != 0)
    {
        fprintf(stderr, "Unable to create requester thread\n");
        exit(1);
    }

    pthread_join(tid1, NULL);

    // TODO: Use a separate instead of thead_args for diff. types
    thread_args->log_path = argv[4];
    if (pthread_create(
            &tid2, NULL,
            resolver_thread_func,
            &thread_args) != 0)
    {
        fprintf(stderr, "Unable to create consumer thread\n");
        exit(1);
    }
    pthread_join(tid2, NULL);

    free(thread_args);
    printf("Parent quiting\n");

    gettimeofday(&t2, NULL);
    sec_elapsed = (float)(t2.tv_sec - t1.tv_sec) +
                  (float)(t2.tv_usec - t1.tv_usec) * 1.0e-6;

    printf("total time is %f seconds\n", sec_elapsed);
    return 0;
}