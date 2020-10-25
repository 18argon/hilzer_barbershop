#ifndef BARBER_H
#define BARBER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.h"

typedef struct barber_t {
    int *id;
    queue_t *cashRegisterQueue;
} barber_t;

barber_t *make_barber(pthread_t *thread, void *(*routine) (void *), int id);

void destroy_barber(barber_t *barber);

#endif