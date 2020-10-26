// Adaptado de https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/
// C program for array implementation of queue
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/queue.h"

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
queue_t *createQueue(unsigned capacity)
{
    queue_t *queue = (queue_t *)malloc(
        sizeof(queue_t));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (int *)malloc(
        queue->capacity * sizeof(int));

    pthread_mutex_init(&(queue->mutex), NULL);
    return queue;
}

// Queue is full when size becomes
// equal to the capacity
int isFull(queue_t *queue)
{
    int size;
    pthread_mutex_lock(&(queue->mutex));
    size = queue->size;
    pthread_mutex_unlock(&(queue->mutex));
    return size == queue->capacity;
}

// Queue is empty when size is 0
int isEmpty(queue_t *queue)
{
    int size;
    pthread_mutex_lock(&(queue->mutex));
    size = queue->size;
    pthread_mutex_unlock(&(queue->mutex));
    return size == 0;
}

// Function to add an item to the queue.
// It changes rear and size
void push(queue_t *queue, int item)
{
    pthread_mutex_lock(&(queue->mutex));
    if (queue->size == queue->capacity) {
        pthread_mutex_unlock(&(queue->mutex));
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    pthread_mutex_unlock(&(queue->mutex));
    // printf("pushed %d\n", item);
}

// Function to remove an item from queue.
// It changes front and size
int pop(queue_t *queue)
{
    pthread_mutex_lock(&(queue->mutex));
    if (queue->size == 0) {
        pthread_mutex_unlock(&(queue->mutex));
        return INT_MIN;
    }
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    pthread_mutex_unlock(&(queue->mutex));
    return item;
}

// Function to get front of queue
int front(queue_t *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}

// Function to get rear of queue
int rear(queue_t *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}

void destroy_queue(queue_t *queue) {
    free(queue->array);
    free(queue);
}
