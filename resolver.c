
#include "resolver.h"

void *resolver_thread_func(void *param)
{
    ThreadInfo *args = (ThreadInfo *)param;
    char *domain = malloc(MAX_DOMAIN_NAME_LENGTH);
    char *ipstr = malloc(INET6_ADDRSTRLEN);
    char *result_line = malloc(INET6_ADDRSTRLEN + MAX_DOMAIN_NAME_LENGTH + 4);

    // find name of this thread
    int short_tid = args->tid % 1000;
    char filename[20] = __FILE__;
    char name[255];
    filename[strlen(filename) - 2] = '\0'; // remove ".c"
    sprintf(name, "%s %d", filename, short_tid);
    printf("in %s\n", name);

    while ((queue_pop(args->shared_buff, domain, name)))
    {
        if (strcmp(domain, "NULL") == 0)
        {
            break;
        }
        dnslookup(domain, ipstr, INET6_ADDRSTRLEN);
        if (sprintf(result_line, "%s, %s\n", domain, ipstr) < 0)
        {

            fputs("Failed to write results", stderr);
            exit(1);
        }
        printf("in %s, resolved: %s", name, result_line);
        queue_push(args->local_buff, result_line, name);
    }

    // fopen a log file for our results
    sem_wait(&args->log_lock);
    FILE *fp = try_fopen(args->log_path, "a", name); // MT-safe
    while (queue_pop(args->local_buff, domain, name))
        fputs(domain, fp);
    fclose(fp); // MT-safe
    sem_post(&args->log_lock);

    printf("in %s: Reached shared buffer end\n", name);
    fflush(stdout); // MT-safe

    free(ipstr);
    free(result_line);
    free(domain);
    return 0;
}