#ifndef _QUEUE_H
#define _QUEUE_H

#include <stddef.h>
#include <stdbool.h>

#include "./utils.h"

struct queue;
typedef struct queue queue_t;

queue_t* new_queue(size_t element_size);
void delete_queue(queue_t *queue);
int queue_push(queue_t *queue, void *value);
int queue_pop(queue_t *queue);
int queue_front(queue_t *queue, void *value);
size_t queue_size(queue_t *queue);
bool queue_empty(queue_t *queue);

#endif // _QUEUE_H
