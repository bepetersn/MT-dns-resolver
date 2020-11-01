
#include "mt-cirque.h"
#include "errno.h"

queue *make_queue(char *name, int size, int mt_safe)
{
    queue *new = malloc(sizeof(queue));
    new->name = name;
    new->head = 0;
    new->tail = 0;
    new->count = 0;
    new->bytes_written = 0;
    new->max_bytes = size * sizeof(*new->data);
    new->capacity = size;
    new->data = malloc(new->max_bytes);
    /* new->data is a pointer to first (which is a) string */
    new->is_mt_safe = mt_safe;
    new->is_bounded = mt_safe; /* Optimization; could change */

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

void _queue_expand_if_needed(queue *q)
{
    void *new_data;
    /* Check if needs to grow */
    printf("%d, %d\n", q->count + 1, q->capacity);
    if ((q->count + 1) >= q->capacity)
    {
        puts("growing");
        q->capacity *= 2;
        new_data = realloc(q->data, sizeof(*q->data) * q->capacity);
        if (new_data == q->data)
        {
            puts("Failed to realloc");
            exit(1);
        }
        else
        {
            q->data = new_data;
            q->max_bytes *= 2;
        }
    }
}

void queue_push(queue *q, char *str, char *caller_name)
{
    if (q->is_mt_safe)
    {
        printf("in %s: trying to acquire space_available for %s (start push)\n",
               q->name, caller_name);
        fflush(stdout); // necessary for some reason
        int val;
        sem_getvalue(&q->space_available, &val);
        printf("in %s: space available: %d for %s (start push)\n",
               q->name, val, caller_name);
        fflush(stdout);                // necessary for some reason
        sem_wait(&q->space_available); // TODO: NOT blocking for some reason?
        sem_wait(&q->mutex);
        printf("in %s: Now acquired space_available for %s (start push)\n",
               q->name, caller_name);
        sem_getvalue(&q->space_available, &val);
        printf("in %s: space available: %d for %s (start push)\n",
               q->name, val, caller_name);
        printf("in %s: going to push '%s'\n", q->name, str);
        q->bytes_written += 1025;
        printf("in %s: bytes written: %d\n", q->name, q->bytes_written);
        printf("in %s: max bytes: %d\n", q->name, q->max_bytes);
        fflush(stdout); // necessary for some reason
    }

    /* Make queue dynamically grow if needed*/
    _queue_expand_if_needed(q);

    // the queue grows from its tail
    strcpy(q->data[q->tail], str);
    q->tail++;
    q->count++;

    if (q->is_mt_safe)
    {
        printf("in %s: signaling items_available for %s (end push)\n",
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
    if (q->tail == q->head)
    {
        // We can't pop from an empty queue
        return NULL;
    }

    char *popped = strdup(q->data[q->head]);
    // We pop from the head of the queue;
    // This has the desirable property that we could in theory
    // pop and push independently from the different
    // sides
    q->head++;
    q->count--;
    q->bytes_written -= 1025;

    if (q->is_mt_safe)
    {
        printf("in %s: signaling space_available for %s (end pop)\n",
               q->name, caller_name);
        sem_post(&q->mutex);
        sem_post(&q->space_available);
    }
    return popped;
}