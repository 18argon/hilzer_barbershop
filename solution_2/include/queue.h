#ifndef QUEUE_H
#define QUEUE_H
// Adaptado de https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/
// C program for array implementation of queue
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// A structure to represent a queue
typedef struct queue_t
{
    int front, rear, size;
    unsigned capacity;
    int *array;
    pthread_mutex_t mutex;
} queue_t;

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
queue_t *createQueue(unsigned capacity);

// Queue is full when size becomes
// equal to the capacity
int isFull(queue_t *queue);

// Queue is empty when size is 0
int isEmpty(queue_t *queue);

// Function to add an item to the queue.
// It changes rear and size
void push(queue_t *queue, int item);

// Function to remove an item from queue.
// It changes front and size
int pop(queue_t *queue);

// Function to get front of queue
int front(queue_t *queue);

// Function to get rear of queue
int rear(queue_t *queue);

void destroy_queue(queue_t *queue);

#endif