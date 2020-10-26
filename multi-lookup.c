
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "mt-cirque.c"
#include "util.h"

#define MAX_INPUT_FILES 10      // TODO: Enforce
#define MAX_REQUESTOR_THREADS 5 // TODO: Enforce
#define MAX_DOMAIN_NAME_LENGTH 1025

void *resolver_thread_func(void *param);
void *requester_thread_func(void *param);

struct ThreadArgs
{
    mt_cirque *file_arr;
    mt_cirque *shared_buff;
    char *log_path;
};

FILE *try_fopen(char *filepath, char *mode, char *caller_name)
{
    FILE *fp = fopen(filepath, mode);
    if (fp == NULL)
    {
        fprintf(stderr, "in %s: failed to open file: %s\n",
                caller_name, filepath);
        exit(1);
    }
    printf("in %s: opened file '%s'\n",
           caller_name, filepath);
    return fp;
}

int main(int argc, char *argv[])
{
    // create an array of files
    // create shared_buffer
    // create logs
    //

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
    printf("# requester log filename: %s\n", argv[3]);
    thread_args->log_path = argv[3];
    printf("# resolver log filename: %s\n", argv[4]);
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
    return 0;
}

void *requester_thread_func(void *param)
{
    printf("in requester\n");

    // Deserialize args
    struct ThreadArgs **argsp = param;
    struct ThreadArgs *args = *argsp;
    char *filepath;
    char *domain = malloc(MAX_DOMAIN_NAME_LENGTH);
    int push_error;

    // Open our logfile
    FILE *log = try_fopen(args->log_path, "w", "requester");

    // While files_arr is not empty, take from files_arr
    while ((filepath = mt_cirque_pop(args->file_arr)) != NULL)
    {

        // open the file from the array
        FILE *fp = try_fopen(filepath, "r", "requester");

        // Read lines from the file repeatedly (MT-safe)
        while (fgets(domain, MAX_DOMAIN_NAME_LENGTH, fp) != NULL &&
               push_error == 0)
        {
            // Remove any newlines that may or may not exist
            domain[strcspn(domain, "\r\n")] = 0; // (MT-safe)
            // printf("in requester: got line from file: %s\n", domain);

            // .. and add each as an entry into the shared buffer
            push_error = mt_cirque_push(args->shared_buff, domain);
            fprintf(log, "%s\n", domain);
            // printf("in requester: added to shared buffer? : %d\n\n", push_error);
        }
        fclose(fp);
    }
    fclose(log);
    free(domain);
    printf("in requester: quiting\n");
    return 0;
}

void *resolver_thread_func(void *param)
{
    printf("in resolver\n");
    struct ThreadArgs **argsp = param;
    struct ThreadArgs *args = *argsp;
    mt_cirque *shared_buff = args->shared_buff;
    char *domain;
    char *ipstr = malloc(INET6_ADDRSTRLEN);
    char *result_line = malloc(INET6_ADDRSTRLEN + MAX_DOMAIN_NAME_LENGTH + 4);

    // fopen and write results out
    FILE *fp = try_fopen(args->log_path, "w", "resolver"); // MT-safe

    while ((domain = mt_cirque_pop(shared_buff)) != NULL)
    {
        dnslookup(domain, ipstr, INET6_ADDRSTRLEN);
        if (sprintf(result_line, "%s, %s\n", domain, ipstr) < 0)
        {

            fputs("Failed to write results", stderr);
            exit(1);
        }
        puts(result_line);
        fputs(result_line, fp);
    }
    fclose(fp);
    puts("in resolver: Reached shared buffer end");
    free(ipstr);
    free(result_line);
    return 0;
}