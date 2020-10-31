
#include "mt-cirque.h"

queue *make_queue(char *name, int size, int mt_safe)
{
    queue *new = malloc(sizeof(queue));
    new->name = name;
    new->is_mt_safe = mt_safe;
    new->head = 0;
    new->count = 0;
    new->tail = 0;
    new->data = malloc(size * sizeof(*new->data));
    /* new->data is a pointer to first (which is a) string */

    if (mt_safe)
    {
        if (sem_init(&new->mutex,
                     0 /* shared between threads */,
                     1 /* Only 1 use at a time */) != 0)
        {
            fputs("Unable to create semaphore\n", stderr);
            exit(1);
        }
        if (sem_init(&new->items_available,
                     0 /* shared between threads */,
                     0 /* Start at 0 items*/) != 0)
        {
            fputs("Unable to create semaphore\n", stderr);
            exit(1);
        }
        if (sem_init(&new->space_available,
                     0 /* shared between threads */,
                     size /* Start at max space */) != 0)
        {
            fputs("Unable to create semaphore\n", stderr);
            exit(1);
        }
    }
    return new;
}

void destroy_queue(queue *q)
{
    if (q->is_mt_safe)
    {
        sem_destroy(&q->items_available);
        sem_destroy(&q->space_available);
        sem_destroy(&q->mutex);
    }
    free(q->data);
    free(q);
}

void queue_display(queue *q)
{
    printf("head -> ");
    int count = q->head + q->count;
    for (int i = q->head; i < count; i++)
    {
        printf("%s (%d) ", q->data[i], i);
        if (i != count - 1)
        {
            printf("-> ");
        }
    }
    puts("<- tail");
}

int queue_has_items_available(queue *q)
{
    int result;
    if (q->is_mt_safe)
        sem_getvalue(&q->items_available, &result);
    else
        result = q->count;
    return result;
}

void queue_push(queue *q, char *str, char *caller_name)
{
    if (q->is_mt_safe)
    {
        printf("in %s: acquiring space_available for %s (start push)\n",
               q->name, caller_name);
        fflush(stdout); // necessary for some reason
        sem_wait(&q->space_available);
        sem_wait(&q->mutex);
    }

    // the queue grows from its tail
    strcpy(q->data[q->tail], str);
    q->tail++;
    q->count++;
    if (q->is_mt_safe)
    {
        printf("in %s: acquiring items_available for %s (end push)\n",
               q->name, caller_name);
        sem_post(&q->mutex);
        sem_post(&q->items_available);
    }
    return;
}

char *queue_pop(queue *q, char *caller_name)
{
    if (q->is_mt_safe)
    {
        printf("in %s: acquiring items_available for %s (start pop)\n",
               q->name, caller_name);
        sem_wait(&q->items_available);
        sem_wait(&q->mutex);
    }

    char *popped = strdup(q->data[q->head]);
    // We pop from the head of the queue;
    // This has the desirable property that we could in theory
    // pop and push independently from the different
    // sides
    q->head++;
    q->count--;
    if (q->is_mt_safe)
    {
        printf("in %s: releasing space_available for %s (end pop)\n",
               q->name, caller_name);
        sem_post(&q->mutex);
        sem_post(&q->space_available);
    }
    return popped;
}