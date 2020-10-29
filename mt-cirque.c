
#include "mt-cirque.h"

mt_cirque *make_mt_cirque()
{
    mt_cirque *new = malloc(sizeof(mt_cirque));
    new->head = 0;
    new->count = 0;
    // NOTE: tail is set to NULL when there
    // are no elements in the queue
    new->tail = UNINITIALIZED;

    if (sem_init(&new->items_available,
                 0 /* shared between threads */,
                 0 /* Only 1 use at a time */) != 0)
    {
        fputs("Unable to create semaphore\n", stderr);
        exit(1);
    }
    if (sem_init(&new->space_available,
                 0 /* shared between threads */,
                 MAX_QUEUE_CAPACITY /* Only 1 use at a time */) != 0)
    {
        fputs("Unable to create semaphore\n", stderr);
        exit(1);
    }

    return new;
}

void mt_cirque_display(mt_cirque *q)
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
    puts("<- tail\n");
}

int mt_cirque_push(mt_cirque *q, char *str, char *caller_name)
{
    printf("in %s: acquiring space_available\n", caller_name);
    fflush(stdout);
    sem_wait(&q->space_available);
    if ((q->tail + 1) % MAX_QUEUE_CAPACITY == q->head)
    {
        // No space remaining, failure
        return -1;
    }
    else if (q->tail == UNINITIALIZED)
    {
        // Queue was previously empty,
        // so we will push at the head position
        q->tail = q->head;
    }
    else
    {
        // Queue was not previously empty,
        // so the queue grows from its tail;
        // the space created is where we push
        q->tail = (q->tail + 1) % MAX_QUEUE_CAPACITY;
    }

    strcpy(q->data[q->tail], str);
    q->count++;
    printf("in %s: releasing items_available\n", caller_name);
    sem_post(&q->items_available);
    return 0;
}

int mt_cirque_has_items_available(mt_cirque *q)
{
    int result;
    sem_getvalue(&q->items_available, &result);
    return result;
}

char *mt_cirque_pop(mt_cirque *q, char *caller_name)
{
    char *popped;
    printf("in %s: acquiring items_available\n", caller_name);
    sem_wait(&q->items_available);
    if (q->tail == UNINITIALIZED)
    {
        // Failure, because we can't pop
        // from an empty queue
        return NULL;
    }
    else
    {
        popped = strdup(q->data[q->head]);
        if (q->head == q->tail)
        {
            // Uninitalize the tail
            // to put the queue in a state
            // of being empty, the inverse
            // of the push functionality above
            q->tail = UNINITIALIZED;
        }
        else
        {
            // We pop from the head of the queue;
            // This has the desirable property that we can
            // pop and push independently from the different
            // sides AND we don't have to reinitialize or
            // grow the queue to accommodate more than its
            // size elements over time
            q->head = (q->head + 1) % MAX_QUEUE_CAPACITY;
        }
        q->count--;
        printf("in %s: releasing space_available\n", caller_name);
        sem_post(&q->space_available);
        return popped;
    }
}