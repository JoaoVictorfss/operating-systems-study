#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

void queue_create(Queue *q) {
    q->front = 0;
    q->position = MAX_SIZE - 1;
    q->size = 0;
}

int queue_push(Queue *q, int val) {
    if (queue_full(q))
        return QUEUE_IS_FULL;

    q->position = (q->position + 1) % MAX_SIZE;
    q->data[q->position] = val;
    q->size++;

    return SUCCESS;
}

int queue_get(Queue *q) {
    if (queue_empty(q))
        return NULL_POINTER_EXCEPTION;

    int val = q->data[q->front];
    return val;
}

int queue_pop(Queue *q) {
    if (queue_empty(q))
        return NULL_POINTER_EXCEPTION;

    q->front = (q->front + 1) % MAX_SIZE;
    q->size--;
}

int queue_full(Queue *q) {
    return q->size == MAX_SIZE;
}

int queue_empty(Queue *q) {
    return q->size == 0;
}

int queue_size(Queue *q) {
    return q->size;
}
