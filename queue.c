// Adaptado de https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/
// C program for array implementation of queue
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

// A structure to represent a queue
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    sem_t **array;
};

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue *createQueue(unsigned capacity)
{
    struct Queue *queue = (struct Queue *)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (sem_t **)malloc(
        queue->capacity * sizeof(sem_t *));
    return queue;
}

// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue *queue)
{
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue *queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
void push(struct Queue *queue, sem_t *item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

// Function to remove an item from queue.
// It changes front and size
sem_t *pop(struct Queue *queue)
{
    if (isEmpty(queue))
        return NULL;
    sem_t *item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
sem_t *front(struct Queue *queue)
{
    if (isEmpty(queue))
        return NULL;
    return queue->array[queue->front];
}

// Function to get rear of queue
sem_t *rear(struct Queue *queue)
{
    if (isEmpty(queue))
        return NULL;
    return queue->array[queue->rear];
}
