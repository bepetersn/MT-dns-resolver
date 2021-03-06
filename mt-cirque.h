
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>

/* prototypes and definitions for a
   multi-threading safe circular 
   queue with a max capcity of 20 */

#define MAX_STRING_LENGTH 1025    // characters in each string
#define DEFAULT_QUEUE_CAPACITY 20 // queue capacity
#define UNINITIALIZED -1000

typedef struct
{
    char *name;
    char (*data)[MAX_STRING_LENGTH]; // declare a pointer that
                                     // can point to whole array
    int head;
    int tail;
    int count;
    int capacity;
    int is_mt_safe;
    int is_one_use_only;
    sem_t items_available;
    sem_t space_available;
    sem_t mutex;
} queue;

queue *make_queue(char *name, int size, int mt_safe, int is_one_use_only);
void destroy_queue(queue *q);
void queue_push(queue *q, char *str, char *caller_name);
char *queue_pop(queue *q, char *result, char *caller_name);
int queue_has_items_available(queue *q);
void queue_display(queue *q);

#define UNUSED(x) (void)(x)
