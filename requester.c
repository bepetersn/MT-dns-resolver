
#include "requester.h"

void *requester_thread_func(void *param)
{
    printf("in requester\n");

    // Deserialize args
    ThreadInfo *args = (ThreadInfo *)param;

    char *filepath;
    char *domain = malloc(MAX_DOMAIN_NAME_LENGTH);
    int push_error;

    // Open our logfile
    FILE *log = try_fopen(args->log_path, "w", "requester");

    // While files_arr is not empty, take from files_arr
    while ((filepath = mt_cirque_pop(args->file_arr, "requester")))
    {

        // open the file from the array
        FILE *fp = try_fopen(filepath, "r", "requester");

        // Read lines from the file repeatedly (MT-safe)
        while ((fgets(domain, MAX_DOMAIN_NAME_LENGTH, fp)))
        {
            printf("in requester: got line from file: %s\n", domain);
            if (push_error == 0)
            {
                printf("pushed %s\n", domain);
                // Remove any newlines that may or may not exist
                domain[strcspn(domain, "\r\n")] = 0; // (MT-safe)

                push_error = mt_cirque_push(args->shared_buff, domain, "requester");
                // For each line, when there is space available,
                // add the domain to the shared buffer
                fprintf(log, "%s\n", domain);
            }
        }
        fclose(fp);
    }
    fclose(log);
    free(domain);
    printf("in requester: quitting\n");
    return 0;
}