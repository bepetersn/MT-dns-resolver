
#include "resolver.h"

void *resolver_thread_func(void *param)
{
    printf("in resolver\n");

    ThreadInfo *args = (ThreadInfo *)param;
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