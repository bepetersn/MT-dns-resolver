
#include "resolver.h"

void *resolver_thread_func(void *param)
{

    ThreadInfo *args = (ThreadInfo *)param;
    char *domain;
    char *ipstr = malloc(INET6_ADDRSTRLEN);
    char *result_line = malloc(INET6_ADDRSTRLEN + MAX_DOMAIN_NAME_LENGTH + 4);

    int short_tid = args->tid % 1000;
    char name[255];
    sprintf(name, "resolver %d", short_tid);
    printf("in %s\n", name);

    // fopen a log file for our results
    FILE *fp = try_fopen(args->log_path, "w", name); // MT-safe

    while ((domain = queue_pop(args->shared_buff, name)))
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
        printf("resolved: %s", result_line);
        fputs(result_line, fp);
    }

    fclose(fp);
    printf("in %s: Reached shared buffer end\n", name);
    fflush(stdout);
    free(ipstr);
    free(result_line);
    return 0;
}