
// header /////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGTH 1025 // characters in string
#define MAX_QUEUE_CAPACITY 20  // queue string capacity

typedef struct
{
    char data[MAX_QUEUE_CAPACITY][MAX_STRING_LENGTH];
    int head;
    int tail;
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
    // NOTE: tail is set to NULL when there
    // are no elements in the queue
    new->tail = -1;
    return new;
}

int mt_cirque_push(mt_cirque *q, char *str)
{
    if ((q->tail + 1) % MAX_QUEUE_CAPACITY == q->head)
    {
        // No space remaining, failure
        return -1;
    }
    else if (q->tail == -1)
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

    char *data[] = q->data;
    data[q->tail] = strdup(str);

    return 0;
}

char *mt_cirque_pop(mt_cirque *q)
{
    if (q->tail == -1)
    {
        // Failure, because we can't pop
        // from an empty queue
        return NULL;
    }
    else
    {
        // We pop from the head of the queue;
        // we achieve this by shrinking the head
        // backwards toward the tail
        q->head++;
    }
    char *data[] = q->data;
    char *str = data[q->head];
    return strdup(str);
}
