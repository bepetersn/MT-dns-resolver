
// Common requirements of requester, resolver
#include "multi-lookup.h"
// Requirements of just multi-lookup
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include "requester.h"
#include "resolver.h"

int main(int argc, char *argv[])
{
    struct timeval t1;
    struct timeval t2;
    float sec_elapsed;
    gettimeofday(&t1, NULL);

    // TODO: write this to performance.txt
    printf("# requesters for this run: %s\n", argv[1]);
    printf("# resolvers for this run: %s\n", argv[2]);

    /* Create shared resources */
    mt_cirque *file_arr = make_mt_cirque("file arr");
    mt_cirque *shared_buff = make_mt_cirque("shared buff");
    for (int arg_index = 5; arg_index < argc; arg_index++)
    {
        printf("queuing %s\n", argv[arg_index]);
        mt_cirque_push(file_arr, argv[arg_index], "main");
    }

    /* Create threads */
    int all_thread_index = 0;
    ThreadInfo *all_thread_infos[MAX_REQUESTOR_THREADS + MAX_RESOLVER_THREADS];

    /* Requesters */
    for (int req_thread_index = 1;
         req_thread_index <= atoi(argv[1]); req_thread_index++)
    {
        ThreadInfo *req_tinfo = init_thread(
            file_arr, shared_buff, argv[3],
            &requester_thread_func, "requester");
        printf("requester #%d\n", req_thread_index);
        all_thread_infos[all_thread_index] = req_tinfo;
        all_thread_index++;
    }

    /* Resolvers */
    for (int res_thread_index = 1;
         res_thread_index <= atoi(argv[2]); res_thread_index++)
    {
        ThreadInfo *res_tinfo = init_thread(
            file_arr, shared_buff, argv[4],
            &resolver_thread_func, "resolver");
        printf("resolver #%d\n", res_thread_index);
        all_thread_infos[all_thread_index] = res_tinfo;
        all_thread_index++;
    }

    /* Wait and clean up */
    for (int thread_index = 0;
         thread_index < all_thread_index; thread_index++)
    {
        printf("thread finished: %ld\n", all_thread_infos[thread_index]->tid % 1000);
        pthread_join(all_thread_infos[thread_index]->tid, NULL);
        // free(all_thread_infos[thread_index]);
    }

    printf("Parent quitting\n");

    gettimeofday(&t2, NULL);
    sec_elapsed = (float)(t2.tv_sec - t1.tv_sec) +
                  (float)(t2.tv_usec - t1.tv_usec) * 1.0e-6;

    printf("total time is %f seconds\n", sec_elapsed);
    return 0;
}

ThreadInfo *init_thread(mt_cirque *file_arr,
                        mt_cirque *shared_buff,
                        char *log_path,
                        thread_func_p thread_func_p,
                        char *caller_name)
{

    // create & serialize pthread_create args
    pthread_t tid;
    ThreadInfo *t_info = malloc(sizeof(ThreadInfo));

    t_info->file_arr = file_arr;
    t_info->shared_buff = shared_buff;
    t_info->log_path = log_path;

    // create a thread for parsing
    if (pthread_create(
            &tid, NULL,
            *thread_func_p,
            t_info) != 0)
    {
        fprintf(stderr, "Unable to create %s thread\n", caller_name);
        // free(t_info);
        exit(1);
    }
    t_info->tid = tid;
    return t_info;
}