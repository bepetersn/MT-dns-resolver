
/* Common requirements / declarations 
   for requester and resolver */

#ifndef MULTI_LOOKUP_HEADER
#define MULTI_LOOKUP_HEADER

#include "util.h"
#include "mt-cirque.h"

#define MAX_INPUT_FILES 10      // TODO: Enforce
#define MAX_REQUESTOR_THREADS 5 // TODO: Enforce
#define MAX_DOMAIN_NAME_LENGTH 1025

struct ThreadArgs
{
    mt_cirque *file_arr;
    mt_cirque *shared_buff;
    char *log_path;
};

#endif