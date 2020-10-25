#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "../include/customer.h"

customer_t *make_customer(pthread_t *thread, void *(*routine) (void *), int id) {
    customer_t *new_customer = malloc(sizeof(customer_t));
    new_customer->id = malloc(sizeof(int));
    *new_customer->id = id;
    if (pthread_create(thread, NULL, routine, (void *) new_customer->id) != 0)
    {
        perror("Problema na criacao da thread\n");
        exit(EXIT_FAILURE);
    }
    sem_init(&new_customer->sem, 0, 0);

    return new_customer;
}

void wait_customer(customer_t *customer) {
    sem_wait(&customer->sem);
}

void signal_customer(customer_t *customer) {
    sem_post(&customer->sem);
}

int get_id(customer_t *customer) {
    return *customer->id;
}

void destroy_customer(customer_t *customer) {
    sem_destroy(&customer->sem);
    free(customer);
}
