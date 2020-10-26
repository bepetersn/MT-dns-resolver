
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGTH 1025 // characters in each string
#define MAX_QUEUE_CAPACITY 20  // queue capacity
#define UNINITIALIZED -1000

typedef struct
{
    char data[MAX_QUEUE_CAPACITY][MAX_STRING_LENGTH];
    int head;
    int tail;
    int count;
} mt_cirque;

int mt_cirque_push();
char *mt_cirque_pop();
void mt_cirque_display();
mt_cirque *make_mt_cirque();