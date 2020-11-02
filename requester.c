
#include "requester.h"

void *requester_thread_func(void *param)
{
    ThreadInfo *args = (ThreadInfo *)param;
    int rv;

    char *filepath = malloc(255);
    char domain[MAX_DOMAIN_NAME_LENGTH];
    FILE *log;
    int num_files_serviced = 0;

    // find name of this thread
    int short_tid = args->tid % 1000;
    char filename[20] = __FILE__;
    char name[255];
    filename[strlen(filename) - 2] = '\0'; // remove ".c"
    rv = sprintf(name, "%s %d", filename, short_tid);
    if (rv < 0)
    {
        fprintf(stderr, "Error in sprintf");
    }
    printf("in %s\n", name);

    // While there is at least one file to process
    while (queue_pop(args->file_arr, filepath, name))
    {
        // TODO: Why am I getting this weird 1 character filepath?
        if (strlen(filepath) == 1)
        {
            printf("1 weird character filepath\n");
            break;
        }
        // Open the file from the array
        FILE *fp = try_fopen(filepath, "r", name, 1);
        if (fp == NULL)
        {
            continue;
        }
        // Read lines from the file repeatedly (MT-safe)
        while ((fgets(domain, MAX_DOMAIN_NAME_LENGTH, fp)))
        {
            // Remove any newlines that may or may not exist
            domain[strcspn(domain, "\r\n")] = 0; // (MT-safe)
            printf("in %s: requesting %s\n", name, domain);

            // Add each as an entry into the shared buffer
            queue_push(args->shared_buff, domain, name);
            // Log what we're doing through this other buffer too
            queue_push(args->local_buff, domain, name);
        }
        num_files_serviced++;
        rv = fclose(fp); // finished one file
        if (rv == EOF)
        {
            fprintf(stderr, "Error in fclose");
        }
    }
    free(filepath);
    puts("Requester reached end of file_arr");

    /* Send a "poison pill" through the shared_buff */
    for (int listener_num = 0;
         listener_num < args->res_req_ratio; listener_num++)
    {
        puts("pushing poison pill");
        queue_push(args->shared_buff, POISON_PILL, name);
    }

    // Write to our logfile
    rv = sem_wait(&args->log_lock);
    if (rv == -1)
    {
        fprintf(stderr, "Error in sprintf");
        exit(1);
    }

    log = try_fopen(args->log_path, "a", name, 0);
    while (queue_pop(args->local_buff, domain, name))
        fprintf(log, "%s\n", domain);

    fprintf(log, "Thread %lu serviced %d files.\n",
            args->tid, num_files_serviced);
    fprintf(args->perf_fp, "Thread %lu serviced %d files.\n",
            args->tid, num_files_serviced);
    rv = fclose(log);
    if (rv == EOF)
    {
        fprintf(stderr, "Error in fclose");
    }

    rv = sem_post(&args->log_lock);
    if (rv == -1)
    {
        fprintf(stderr, "Error in sem_post");
        exit(1);
    }

    return 0;
}