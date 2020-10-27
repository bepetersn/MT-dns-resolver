
#include "requester.h"

void *requester_thread_func(void *param)
{
    printf("in requester\n");

    // Deserialize args
    ThreadInfo *args = (ThreadInfo *)param;

    char filepath[MAX_DOMAIN_NAME_LENGTH];
    char *domain = malloc(MAX_DOMAIN_NAME_LENGTH);
    int push_error;

    // Open our logfile
    FILE *log = try_fopen(args->log_path, "w", "requester");

    // While files_arr is not empty, take from files_arr
    for (int file_index = 0; file_index < MAX_INPUT_FILES; file_index++)
    {
        strcpy(filepath, args->file_arr[file_index]);
        if (!strlen(filepath))
        {
            break;
        }
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
            printf("%s\n", domain);
            fprintf(log, "%s\n", domain);
            // printf("in requester: added to shared buffer? : %d\n\n", push_error);
        }
        fclose(fp);
    }
    fclose(log);
    free(domain);
    printf("in requester: quitting\n");
    return 0;
}