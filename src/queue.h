#ifndef _QUEUE_H
#define _QUEUE_H

#include <stddef.h>

typedef struct node
{
    void * value;
    struct node * next;
} node_t;


typedef struct queue
{
    node_t * front;
    node_t * back;
    size_t current_size;
    size_t max_size;
} queue_t;

void initQueue(queue_t * q, size_t max_size);
void push(queue_t * q, void * value);
void pop(queue_t * q);
void * front(queue_t q);

#endif