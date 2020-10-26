
#ifndef MULTI_LOOKUP_HEADER
#define MULTI_LOOKUP_HEADER

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "mt-cirque.h"

#define MAX_INPUT_FILES 10      // TODO: Enforce
#define MAX_REQUESTOR_THREADS 5 // TODO: Enforce
#define MAX_DOMAIN_NAME_LENGTH 1025

void *resolver_thread_func(void *param);
FILE *try_fopen(char *filepath, char *mode, char *caller_name);

struct ThreadArgs
{
    mt_cirque *file_arr;
    mt_cirque *shared_buff;
    char *log_path;
};

#endif