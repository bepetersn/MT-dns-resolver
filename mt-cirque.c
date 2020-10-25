
// header /////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGTH 1025 // characters in each string
#define MAX_QUEUE_CAPACITY 100 // queue capacity
#define UNINITIALIZED -1000

typedef struct
{
    char data[MAX_QUEUE_CAPACITY][MAX_STRING_LENGTH];
    int head;
    int tail;
    int count;
} mt_cirque;

void push();
void pop();
void display();
mt_cirque *make_mt_cirque();

///////////////////////////////////////////////////////

mt_cirque *make_mt_cirque()
{
    mt_cirque *new = malloc(sizeof(mt_cirque));
    new->head = 0;
    new->count = 0;
    // NOTE: tail is set to NULL when there
    // are no elements in the queue
    new->tail = UNINITIALIZED;
    return new;
}

void mt_cirque_display(mt_cirque *q)
{
    printf("head:%s", " ");
    for (int i = q->head; i < q->count; i++)
    {
        printf("'%s' -> ", q->data[i]);
    }
}

int mt_cirque_push(mt_cirque *q, char *str)
{
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
    return 0;
}

char *mt_cirque_pop(mt_cirque *q)
{
    char *popped;
    mt_cirque_display(q);
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
        return popped;
    }
}