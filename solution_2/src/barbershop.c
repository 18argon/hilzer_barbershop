#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "../include/queue.h"
#include "../include/customer.h"
#include "../include/barber.h"

#define N_BARBERS 3
#define N_CUSTOMERS 25
#define BARBERSHOP_CAPACITY 20

void *barber_routine(void *args);
void *customer_routine(void *args);

pthread_mutex_t door, sitting, cashRegister;
sem_t sofa, barber, anyoneOnSofa;

int customers_count = 0;
queue_t *sofaQueue;

int queueBeingAtended = 1;
queue_t *cashRegisterQueues[N_BARBERS];
int cashRegisterIsOccupied = 0;

barber_t *barbers[N_BARBERS];
customer_t *customers[N_CUSTOMERS];

void initialize_mutexes() {
    pthread_mutex_init(&door, NULL);
    pthread_mutex_init(&sitting, NULL);
    pthread_mutex_init(&cashRegister, NULL);
}

void destroy_mutexes() {
    pthread_mutex_destroy(&door);
    pthread_mutex_destroy(&sitting);
    pthread_mutex_destroy(&cashRegister);
}

void initialize_semaphores() {
    sem_init(&sofa, 0, N_BARBERS);
    sem_init(&barber, 0, N_BARBERS);
    sem_init(&anyoneOnSofa, 0, 0);
}

void destroy_semaphores() {
    sem_destroy(&sofa);
    sem_destroy(&barber);
    sem_destroy(&anyoneOnSofa);
}

int main()
{
    int i;
    pthread_t barbers_threads[N_BARBERS], customers_threads[N_CUSTOMERS];
    
    sofaQueue = createQueue(N_BARBERS);

    initialize_mutexes();
    initialize_semaphores();

    for (i = 0; i < N_BARBERS; i++)
    {
        int id = i;
        barbers[i] = make_barber(&barbers_threads[i], barber_routine, id);
    }

    for (i = 0; i < N_CUSTOMERS; i++)
    {
        int id = i;
        customers[i] = make_customer(&customers_threads[i], customer_routine, id);
    }

    for (i = 0; i < N_CUSTOMERS; i++)
    {
        if (pthread_join(customers_threads[i], NULL) != 0)
        {
            perror("Problema no join\n");
            exit(EXIT_FAILURE);
        }
    }

    sleep(1);
    printf("Todos os clientes foram atendidos\n");
    
    for (i = 0; i < N_BARBERS; i++)
    {
        printf("Barbeiro #%d encerrou o seu expediente\n", i);
        pthread_cancel(barbers_threads[i]);
    }

    // Liberando recursos
    for (i = 0; i < N_BARBERS; i++) {
        destroy_barber(barbers[i]);
    }

    for (i = 0; i < N_CUSTOMERS; i++) {
        destroy_customer(customers[i]);
    }

    destroy_queue(sofaQueue);

    destroy_mutexes();
    destroy_semaphores();

    return 0;
}

void *barber_routine(void *args)
{
    int id = *((int *) args);
    unsigned int customer_id;
    customer_t *customer;
    printf("Barbeiro #%d iniciou o expediente\n", id);
    while (1)
    {
        pthread_mutex_lock(&cashRegister);
        if (cashRegisterIsOccupied == 0) {
            cashRegisterIsOccupied = 1;
            pthread_mutex_unlock(&cashRegister);
            while(!isEmpty(barbers[id]->cashRegisterQueue)) {
                customer_id = pop(barbers[id]->cashRegisterQueue);
                customer = customers[customer_id];
                printf("Barbeiro #%d cobrou o cliente #%d\n", id, customer_id);
                sem_post(&(customer->sem));
            }
            pthread_mutex_lock(&cashRegister);
            cashRegisterIsOccupied = 0;
        } 
        pthread_mutex_unlock(&cashRegister);

        sem_post(&barber);
        sem_wait(&anyoneOnSofa);

        pthread_mutex_lock(&sitting);
        customer_id = pop(sofaQueue);
        customer = customers[customer_id];
        pthread_mutex_unlock(&sitting);

        sem_post(&(customer->sem));
        // inicia o atendimento
        sleep(1);
        // atendimento finalizado
        sem_post(&(customer->sem));
        push(barbers[id]->cashRegisterQueue, get_id(customer));
    }
}

void *customer_routine(void *arg)
{
    int id = *((int *) arg);

    pthread_mutex_lock(&door);

    if (customers_count == BARBERSHOP_CAPACITY) {
        pthread_mutex_unlock(&door);
        printf("Loja cheia! Cliente #%d foi embora.\n", id);
        pthread_exit((void *) EXIT_SUCCESS);
    }
    printf("Cliente #%d entrou na loja.\n", id);
    customers_count++;
    pthread_mutex_unlock(&door);

    sem_wait(&sofa);
    printf("Cliente #%d sentou no sofa.\n", id);
    
        pthread_mutex_lock(&sitting);
            push(sofaQueue, id);
        pthread_mutex_unlock(&sitting);
    // sem_post(&customer);

        sem_post(&anyoneOnSofa);
        sem_wait(&barber);
        sem_wait(&(customers[id]->sem));
    sem_post(&sofa);
    printf("Cliente #%d esta sendo atendido.\n", id);

    // Esperando terminar o atendimento
    sem_wait(&(customers[id]->sem));

    printf("Cliente #%d foi atendido.\n", id);

    // pthread_mutex_lock(&register);

    // Esperando o pagamento;
    printf("Cliente #%d esta esperando para pagar.\n", id);
    sem_wait(&(customers[id]->sem));
    printf("Cliente #%d foi embora.\n", id);

    pthread_mutex_lock(&door);
    customers_count--;
    pthread_mutex_unlock(&door);

    pthread_exit((void *) 0);
}
