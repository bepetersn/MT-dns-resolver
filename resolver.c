
#include "resolver.h"

void *resolver_thread_func(void *param)
{
    ThreadInfo *args = (ThreadInfo *)param;
    int rv;
    char *domain = malloc(MAX_DOMAIN_NAME_LENGTH);
    if (domain == NULL)
    {
        fprintf(stderr, "Error in malloc");
        exit(1);
    }
    char *ipstr = malloc(INET6_ADDRSTRLEN + 1);
    if (ipstr == NULL)
    {
        fprintf(stderr, "Error in malloc");
        exit(1);
    }
    char *result_line = malloc(INET6_ADDRSTRLEN + MAX_DOMAIN_NAME_LENGTH + 4);
    if (result_line == NULL)
    {
        fprintf(stderr, "Error in malloc");
        exit(1);
    }

    // find name of this thread
    int short_tid = args->tid % 1000;
    char filename[20] = __FILE__;
    char name[255];
    filename[strlen(filename) - 2] = '\0'; // remove ".c"
    rv = sprintf(name, "%s %x", filename, short_tid);
    if (rv < 0)
    {
        fprintf(stderr, "Error in sprintf");
    }
    // printf("in %s\n", name);

    while (queue_pop(args->shared_buff, domain, name))
    {
        if (strcmp(domain, POISON_PILL) == 0)
        {
            break;
        }
        rv = dnslookup(domain, ipstr, INET6_ADDRSTRLEN + 1);
        if (rv == -1)
        {
            fprintf(stderr, "Bogus hostname: %s\n", domain);
            rv = sprintf(result_line, "%s,\n", domain);
            if (rv < 0)
            {
                fprintf(stderr, "Error in sprintf");
                exit(1);
            }
        }
        else
        {
            rv = sprintf(result_line, "%s, %s\n", domain, ipstr);
            if (rv < 0)
            {
                fprintf(stderr, "Error in sprintf");
                exit(1);
            }
        }
        // printf("in %s, resolved: %s", name, result_line);
        queue_push(args->local_buff, result_line, name);
    }

    // fopen a log file for our results
    rv = sem_wait(&args->log_lock);
    if (rv < 0)
    {
        fprintf(stderr, "Error in sem_wait");
        exit(1);
    }
    FILE *fp = try_fopen(args->log_path, "a", name, 0); // MT-safe
    while (queue_pop(args->local_buff, domain, name))
    {
        rv = fputs(domain, fp);
        if (rv == EOF)
        {
            fprintf(stderr, "Error in fputs");
            exit(1);
        }
    }
    rv = fclose(fp);
    if (rv == EOF)
    {
        fprintf(stderr, "Error in fclose");
        exit(1);
    }
    rv = sem_post(&args->log_lock);
    if (rv == -1)
    {
        fprintf(stderr, "Error in sem_post");
        exit(1);
    }

    // printf("in %s: Reached shared buffer end\n", name);
    rv = fflush(stdout); // MT-safe
    if (rv == EOF)
    {
        fprintf(stderr, "Error in fflush");
        exit(1);
    }

    free(ipstr);
    free(result_line);
    free(domain);
    // puts("reached end of thread");
    return 0;
}