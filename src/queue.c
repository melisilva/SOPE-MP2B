#include "stdlib.h"
#include <stdbool.h>

#include "queue.h"

void initQueue(queue_t * q, size_t max_size){
    q->max_size = max_size;
    q->current_size = 0;
    q->front = NULL;
    q->back = NULL;
}

void push(queue_t * q, int* value){

    q->current_size++;

    if (q->front == NULL){
        node_t * new_node = malloc(sizeof(node_t));
        new_node->value = value;
        new_node->next = NULL;
        q->front = new_node;
        q->back = new_node;
        return;
    }

    node_t * new_node = malloc(sizeof(node_t));
    new_node->value = value;
    new_node->next = NULL;
    q->back->next = new_node;
    q->back = new_node;
}

void pop(queue_t * q){
    if (q->front == NULL) return;

    node_t * old_node = q->front;
    q->front = old_node->next;

    free(old_node->value);
    free(old_node);

    q->current_size--;
}

void * front(queue_t q){
    return q.front->value;
}

bool empty(queue_t* q){
    return (q->current_size == 0);
}

bool full(queue_t *q) {
    return (q->current_size == q->max_size);
}