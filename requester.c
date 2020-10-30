
#include "requester.h"

void *requester_thread_func(void *param)
{
    ThreadInfo *args = (ThreadInfo *)param;

    char *filepath;
    char domain[MAX_DOMAIN_NAME_LENGTH];
    FILE *log;
    int push_error;

    int short_tid = args->tid % 1000;
    char name[255];
    sprintf(name, "requester %d", short_tid);
    printf("in %s\n", name);

    // While files_arr is not empty, take from files_arr
    while (mt_cirque_has_items_available(args->file_arr))
    {
        filepath = mt_cirque_pop(args->file_arr, name);
        // printf("filepath len: %lu\n", strlen(filepath));
        // TODO: Why am I getting this weird 1 character filepath?
        if (strlen(filepath) == 1)
        {
            break;
        }
        // Open the file from the array
        FILE *fp = try_fopen(filepath, "r", name);

        // Read lines from the file repeatedly (MT-safe)
        while ((fgets(domain, MAX_DOMAIN_NAME_LENGTH, fp)))
        {
            // Remove any newlines that may or may not exist
            domain[strcspn(domain, "\r\n")] = 0; // (MT-safe)
            printf("requesting %s\n", domain);

            // Add each as an entry into the shared buffer
            mt_cirque_push(args->shared_buff, domain, name);

            // Write to our logfile
            // TODO: Do all of our writing in one go by dynamically allocating memory;
            //       This is too annoying to do right now
            log = try_fopen(args->log_path, "w", name);
            fprintf(log, "%s\n", domain);
            fclose(log);
        }
        fclose(fp); // finished one file
    }
    /* Send a "poison pill" through the shared_buff */
    mt_cirque_push(args->shared_buff, "NULL", name);
    printf("in %s: quitting\n", name);
    return 0;
}