
// header /////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGTH 1025 // characters in string
#define MAX_QUEUE_CAPACITY 20  // queue string capacity
#define UNINITIALIZED -1000

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
    new->tail = UNINITIALIZED;
    return new;
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
    return 0;
}

char *mt_cirque_pop(mt_cirque *q)
{
    if (q->tail == UNINITIALIZED)
    {
        // Failure, because we can't pop
        // from an empty queue
        return NULL;
    }
    else if (q->head == q->tail)
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
        // we achieve this by shrinking the head
        // backwards toward the tail; this sounds
        // weird but it just means the queue gradually
        // shifts its head forward relative to 0 as
        // it is popped, which has the desirable property
        // that it means we never have to re-initialize it
        q->head++;
    }
    return strdup(q->data[q->head]);
}