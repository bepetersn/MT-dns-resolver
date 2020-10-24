
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 20
#define MAX_INPUT_FILES 10
#define MAX_DOMAIN_NAME_LENGTH 1025
#define MAX_REQUESTOR_THREADS 5
#define MAX_IP_LENGTH = 46 // INET6_ADDRSTRLEN?

void *resolver_thread_func(void *param);
void *requester_thread_func(void *param);
int next_ptr = 0;

struct ThreadArgs
{
    char *file_arr[MAX_INPUT_FILES];
    char *shared_buff[ARRAY_SIZE];
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
    char **file_arr = args->file_arr;
    char **shared_buff = args->shared_buff;

    // Given a file ...
    char *buf = malloc(MAX_DOMAIN_NAME_LENGTH);
    char *filepath = file_arr[0];
    char *result;
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL)
    {
        printf("in requester: failed to open file: %s\n", filepath);
        exit(1);
    }
    printf("in requester: opened file '%s'\n", args->file_arr[0]);
    do
    {
        result = fgets(buf, MAX_DOMAIN_NAME_LENGTH, fp);
        printf("in requester: got line from file: %s", buf);

        // .. and add an entry into the shared buffer
        shared_buff[next_ptr] = buf;
        next_ptr++;

        printf("in requester: added to shared buffer\n\n");
    } while (result != NULL && next_ptr <= ARRAY_SIZE);
    printf("in requester: quiting\n");
    return 0;
}
void *resolver_thread_func(void *param)
{
    printf("in resolver\n");
    struct ThreadArgs **argsp = param;
    int i = 0;
    do
    {
        printf("%d: %s\n", i, (*argsp)->shared_buff[i]);
        i++;
    } while (i < next_ptr);
    return 0;
}
