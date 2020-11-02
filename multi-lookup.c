
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
    int rv;
    rv = gettimeofday(&t1, NULL);
    if (rv != 0)
    {
        fprintf(stderr, "Error in gettimeofday");
        exit(1);
    }
    int num_requesters = atoi(argv[1]);
    int num_resolvers = atoi(argv[2]);
    if (num_requesters < 1 || num_resolvers < 1)
    {
        fprintf(stderr, "Error in num threads arguments");
        exit(1);
    }

    if (num_requesters > MAX_REQUESTOR_THREADS)
    {
        fprintf(stderr, "Max requestor threads is %d\n", MAX_REQUESTOR_THREADS);
        exit(1);
    }
    if (num_resolvers > MAX_RESOLVER_THREADS)
    {
        fprintf(stderr, "Max resolver threads is %d\n", MAX_RESOLVER_THREADS);
        exit(1);
    }

    // TODO: write this to performance.txt
    FILE *perf_fp = try_fopen("performance.txt", "a+", "main", 0);

    rv = fprintf(perf_fp, "# requesters for this run: %d\n", num_requesters);
    if (rv < 0)
    {
        fprintf(stderr, "Error writing performance.txt");
        exit(1);
    }
    rv = fprintf(perf_fp, "# resolvers for this run: %d\n", num_resolvers);
    if (rv < 0)
    {
        fprintf(stderr, "Error writing performance.txt");
        exit(1);
    }

    // Ratio of resolvers to requesters, 1 <= rrr <= 5
    int res_req_ratio = (num_resolvers / num_requesters);
    float f_rrr = (((float)num_resolvers) / ((float)num_requesters));
    if ((f_rrr - (float)res_req_ratio) > 0)
        res_req_ratio++;

    /* Create shared resources */
    ///////////////////////////////////////////////
    queue *file_arr = make_queue("file arr", ARRAY_SIZE, 1);
    queue *shared_buff = make_queue("shared buff", MAX_INPUT_FILES, 1);
    sem_t requester_log_lock, resolver_log_lock;

    /* Only one use of log locks at a time, in threads */
    rv = sem_init(&requester_log_lock, 0, 1);
    if (rv == -1)
    {
        fprintf(stderr, "Error in initializing semaphore");
        exit(1);
    }
    rv = sem_init(&resolver_log_lock, 0, 1);
    if (rv == -1)
    {
        fprintf(stderr, "Error in initializing semaphore");
        exit(1);
    }

    /* Empty the log files */
    FILE *fp = try_fopen(argv[3], "w", "main", 0);
    rv = fclose(fp);
    if (rv == EOF)
    {
        fprintf(stderr, "Error in overwriting log file");
        exit(1);
    }
    fp = try_fopen(argv[4], "w", "main", 0);
    fclose(fp);
    if (rv == EOF)
    {
        fprintf(stderr, "Error in overwriting log file");
        exit(1);
    }

    for (int arg_index = 5; arg_index < argc; arg_index++)
    {
        // printf("queuing %s\n", argv[arg_index]);
        queue_push(file_arr, argv[arg_index], "main");
    }
    ///////////////////////////////////////////////

    /* Create threads */
    ////////////////////////////////////
    int all_thread_index = 0;
    ThreadInfo *all_thread_infos[MAX_REQUESTOR_THREADS + MAX_RESOLVER_THREADS];

    /* Requesters */
    for (int req_thread_index = 1;
         req_thread_index <= num_requesters; req_thread_index++)
    {
        ThreadInfo *req_tinfo = init_thread(
            file_arr, shared_buff, requester_log_lock,
            argv[3], &requester_thread_func, res_req_ratio);
        // // printf("Created thread: requester %ld\n", req_tinfo->tid % 1000);
        req_tinfo->perf_fp = perf_fp;
        all_thread_infos[all_thread_index] = req_tinfo;
        all_thread_index++;
    }

    /* Resolvers */
    for (int res_thread_index = 1;
         res_thread_index <= num_resolvers; res_thread_index++)
    {
        ThreadInfo *res_tinfo = init_thread(
            file_arr, shared_buff, resolver_log_lock,
            argv[4], &resolver_thread_func, 0);
        // printf("Created thread: resolver %ld\n", res_tinfo->tid % 1000);
        res_tinfo->perf_fp = perf_fp;
        all_thread_infos[all_thread_index] = res_tinfo;
        all_thread_index++;
    }
    ///////////////////////////////////

    /* Wait and clean up */
    for (int thread_index = 0;
         thread_index < all_thread_index; thread_index++)
    {
        int rv = pthread_join(all_thread_infos[thread_index]->tid, NULL);
        if (rv != 0)
        {
            puts("Failed to join");
            exit(1);
        }
        destroy_queue(all_thread_infos[thread_index]->local_buff);
        free(all_thread_infos[thread_index]);
    }
    destroy_queue(shared_buff);
    destroy_queue(file_arr);

    // printf("Parent quitting\n");

    rv = gettimeofday(&t2, NULL);
    if (rv != 0)
    {
        puts("Failed to gettimeofday");
        exit(1);
    }
    sec_elapsed = (float)(t2.tv_sec - t1.tv_sec) +
                  (float)(t2.tv_usec - t1.tv_usec) * 1.0e-6;

    printf("./multi-lookup: total time is %f seconds\n", sec_elapsed);
    fprintf(perf_fp, "./multi-lookup: total time is %f seconds\n", sec_elapsed);

    rv = fclose(perf_fp);
    if (rv == EOF)
    {
        fprintf(stderr, "Error in closing performance.txt");
        exit(1);
    }
    return 0;
}

ThreadInfo *init_thread(queue *file_arr,
                        queue *shared_buff,
                        sem_t log_lock,
                        char *log_path,
                        thread_func_p thread_func_p,
                        int res_req_ratio)
{

    // create & serialize pthread_create args
    pthread_t tid;
    ThreadInfo *t_info = malloc(sizeof(ThreadInfo));
    if (t_info == NULL)
    {
        puts("Failed to malloc");
        exit(1);
    }

    t_info->file_arr = file_arr;
    t_info->shared_buff = shared_buff;
    t_info->local_buff = make_queue("local buff n", 0, 0); /* Unbounded and not MT-safe */
    t_info->log_lock = log_lock;
    t_info->log_path = log_path;
    t_info->res_req_ratio = res_req_ratio;

    // create a thread for parsing
    if (pthread_create(
            &tid, NULL,
            *thread_func_p,
            t_info) != 0)
    {
        fprintf(stderr, "Unable to create thread\n");
        free(t_info);
        exit(1);
    }
    t_info->tid = tid;
    return t_info;
}
