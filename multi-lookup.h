
/* Common requirements / declarations 
   for requester and resolver, */

#ifndef MULTI_LOOKUP_HEADER
#define MULTI_LOOKUP_HEADER

#include "util.h"
#include "mt-cirque.h"

#define ARRAY_SIZE 20
#define MAX_INPUT_FILES 10      // TODO: Enforce
#define MAX_REQUESTOR_THREADS 5 // TODO: Enforce
#define MAX_RESOLVER_THREADS 10 // TOOD: Enforce
#define MAX_DOMAIN_NAME_LENGTH 1025

#define POISON_PILL "__END_INPUT__"

typedef struct
{
   pthread_t tid;
   queue *file_arr;
   queue *shared_buff;
   queue *local_buff;
   char *log_path;
   sem_t log_lock;
   int res_req_ratio;
   FILE *perf_fp;
} ThreadInfo;

/* Declare a type which is a pointer to a function 
   valid for use as the 3rd parameter to pthread_create */
typedef void *(*thread_func_p)(void *);

ThreadInfo *init_thread(queue *file_arr,
                        queue *shared_buff,
                        sem_t log_lock,
                        char *log_path,
                        thread_func_p thread_func_p,
                        int res_req_ratio);

#endif