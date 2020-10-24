
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "mt-cirque.c"
#include "util.h"

#define ARRAY_SIZE 20
#define MAX_INPUT_FILES 10
#define MAX_DOMAIN_NAME_LENGTH 1025
#define MAX_REQUESTOR_THREADS 5

void *resolver_thread_func(void *param);
void *requester_thread_func(void *param);
int next_ptr = 0;

struct ThreadArgs
{
    char *file_arr[MAX_INPUT_FILES];
    mt_cirque *shared_buff;
    FILE *log;
};

int main(int argc, char *argv[])
{
    // create an array of files
    // create shared_buffer
    // create a parsing log
    //

    puts("in main");

    // create & serialize pthread_create args
    pthread_t tid1, tid2;
    struct ThreadArgs *thread_args = malloc(sizeof(struct ThreadArgs));
    thread_args->file_arr[0] = argv[1];
    thread_args->shared_buff = make_mt_cirque();

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
    if (pthread_create(
            &tid2, NULL,
            resolver_thread_func,
            &thread_args) != 0)
    {
        fprintf(stderr, "Unable to create consumer thread\n");
        exit(1);
    }
    pthread_join(tid2, NULL);

    printf("Parent quiting\n");

    return 0;
}

void *requester_thread_func(void *param)
{
    printf("in requester\n");

    // Deserialize args
    struct ThreadArgs **argsp = param;
    struct ThreadArgs *args = *argsp;

    // Given a file ...
    char *filepath = args->file_arr[0];
    char *domain = malloc(MAX_DOMAIN_NAME_LENGTH);
    int push_error;

    // open it ...
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL)
    {
        printf("in requester: failed to open file: %s\n", filepath);
        exit(1);
    }
    printf("in requester: opened file '%s'\n", args->file_arr[0]);

    // Read lines from the file repeatedly
    while (fgets(domain, MAX_DOMAIN_NAME_LENGTH, fp) != NULL &&
           push_error == 0)
    {
        // Remove any newlines that may or may not exist
        domain[strcspn(domain, "\r\n")] = 0; // (MT-safe)
        printf("in requester: got line from file: %s\n", domain);

        // .. and add each as an entry into the shared buffer
        push_error = mt_cirque_push(args->shared_buff, domain);
        printf("in requester: added to shared buffer? ...: %d\n\n", push_error);
    }
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
    while ((domain = mt_cirque_pop(shared_buff)) != NULL)
    {
        dnslookup(domain, ipstr, INET6_ADDRSTRLEN);
        printf("%s, %s\n", domain, ipstr);
    }
    puts("in resolver: Reached shared buffer end");
    free(ipstr);
    return 0;
}
