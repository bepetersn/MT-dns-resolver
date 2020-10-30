
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

/* prototypes and definitions for a
   multi-threading safe circular 
   queue with a max capcity of 20 */

#define MAX_STRING_LENGTH 1025 // characters in each string
#define MAX_QUEUE_CAPACITY 20  // queue capacity
#define UNINITIALIZED -1000

typedef struct
{
    char *name;
    char data[MAX_QUEUE_CAPACITY][MAX_STRING_LENGTH];
    int head;
    int tail;
    int count;
    sem_t items_available;
    sem_t space_available;
} mt_cirque;

int mt_cirque_push(mt_cirque *q, char *str, char *caller_name);
char *mt_cirque_pop(mt_cirque *q, char *caller_name);
int mt_cirque_has_items_available(mt_cirque *q);
void mt_cirque_display(mt_cirque *q);
mt_cirque *make_mt_cirque(char *name);