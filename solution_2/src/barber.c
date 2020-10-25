#include "../include/barber.h"

barber_t *make_barber(pthread_t *thread, void *(*routine) (void *), int id) {
    barber_t *new_barber = malloc(sizeof(barber_t));
    new_barber->id = malloc(sizeof(int));
    *new_barber->id = id;
    new_barber->cashRegisterQueue = createQueue(10);
    if (pthread_create(thread, NULL, routine, (void *) new_barber->id) != 0)
    {
        perror("Problema na criacao da thread\n");
        exit(EXIT_FAILURE);
    }
    return new_barber;
}

void destroy_barber(barber_t *barber) {
    destroy_queue(barber->cashRegisterQueue);
    free(barber);
}