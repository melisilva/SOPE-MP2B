#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "queue.h"

typedef struct node {
    void *value;
    struct node *next;
} node_t;

struct queue {
    node_t *front;
    node_t *back;
    size_t current_size;
    size_t element_size;
};


queue_t* new_queue(size_t element_size) {
    queue_t *queue = malloc(sizeof(queue_t));
    if (queue == NULL)
        return NULL;
    
    queue->element_size = element_size;
    queue->current_size = 0;
    queue->front = NULL;
    queue->back = NULL;

    return queue;
}


void delete_queue(queue_t *queue) {
    if (queue == NULL)
        return;
    
    while (queue_pop(queue) == 0);
    free(queue);
}


int queue_push(queue_t *queue, void *value){
    if (queue == NULL)
        return 1;

    node_t *new_node = malloc(sizeof(node_t));
    if (new_node == NULL)
        return 1;

    
    new_node->value = malloc(queue->element_size);
    if (new_node->value == NULL)
        return 1;

    memcpy(new_node->value, value, queue->element_size);
    queue->current_size++;
    new_node->next = NULL;

    if (queue->front == NULL){
        queue->front = new_node;
        queue->back = new_node;

    } else {
        queue->back->next = new_node;
        queue->back = new_node;
    }

    return 0;
}


int queue_pop(queue_t * queue){
    if (queue == NULL)
        return 1;

    if (queue->front == NULL) 
        return 1;

    node_t *old_node = queue->front;
    queue->front = queue->front->next;

    free(old_node->value);
    free(old_node);

    queue->current_size--;

    return 0;
}

int queue_front(queue_t *queue, void *value) {
     if (queue == NULL)
         return 1;
     if (value == NULL)
         return 1;

     memcpy(value, queue->front->value, queue->element_size);

     return 0;
}

bool queue_empty(queue_t *queue){
    if (queue == NULL)
        return true; // ?
    
    return queue->current_size == 0;
}
