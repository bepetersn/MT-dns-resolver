
/* Common requirements / declarations 
   for requester and resolver */

#ifndef MULTI_LOOKUP_HEADER
#define MULTI_LOOKUP_HEADER

#include "util.h"
#include "mt-cirque.h"

#define MAX_INPUT_FILES 10      // TODO: Enforce
#define MAX_REQUESTOR_THREADS 5 // TODO: Enforce
#define MAX_DOMAIN_NAME_LENGTH 1025

typedef struct
{
   pthread_t tid;
   char file_arr[MAX_INPUT_FILES][MAX_DOMAIN_NAME_LENGTH];
   mt_cirque *shared_buff;
   char *log_path;
} ThreadInfo;

/* Declare a type which is a pointer to a function 
   valid for use as the 3rd parameter to pthread_create */
typedef void *(*thread_func_p)(void *);

ThreadInfo *init_thread(char file_arr[MAX_INPUT_FILES][MAX_DOMAIN_NAME_LENGTH],
                        mt_cirque *shared_buff,
                        char *log_path,
                        thread_func_p thread_func_p,
                        char *caller_name);

#endif