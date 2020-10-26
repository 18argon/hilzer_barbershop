#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct customer_t {
    sem_t sem;
    int *id;
} customer_t;

customer_t *make_customer(int id);

void wait_customer(customer_t *customer);

void signal_customer(customer_t *customer);

int get_id(customer_t *customer);

void destroy_customer(customer_t *customer);

#endif