
#include "requester.h"

void *requester_thread_func(void *param)
{
    ThreadInfo *args = (ThreadInfo *)param;

    char *filepath = malloc(255);
    char domain[MAX_DOMAIN_NAME_LENGTH];
    FILE *log;

    // find name of this thread
    int short_tid = args->tid % 1000;
    char filename[20] = __FILE__;
    char name[255];
    filename[strlen(filename) - 2] = '\0'; // remove ".c"
    sprintf(name, "%s %d", filename, short_tid);
    printf("in %s\n", name);

    // While there is at least one file to process
    while (queue_has_items_available(args->file_arr))
    {
        queue_pop(args->file_arr, filepath, name);
        // TODO: Why am I getting this weird 1 character filepath?
        if (strlen(filepath) == 1)
        {
            printf("1 weird character filepath\n");
            break;
        }
        // Open the file from the array
        FILE *fp = try_fopen(filepath, "r", name);
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
        fclose(fp); // finished one file
    }
    free(filepath);

    // Write to our logfile
    sem_wait(&args->log_lock);
    log = try_fopen(args->log_path, "a", name);
    while (queue_pop(args->local_buff, domain, name))
        fprintf(log, "%s\n", domain);
    fclose(log);
    sem_post(&args->log_lock);

    /* Send a "poison pill" through the shared_buff */
    for (int listener_num = 0;
         listener_num < args->res_req_ratio; listener_num++)
        queue_push(args->shared_buff, "NULL", name); // TODO: Don't send "NULL" -- send NULL

    printf("in %s: quitting\n", name);
    return 0;
}