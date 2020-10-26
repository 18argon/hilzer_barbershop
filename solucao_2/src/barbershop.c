#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "../include/queue.h"
#include "../include/customer.h"
#include "../include/barber.h"

#define TRUE 1
#define FALSE 0

#define N_BARBERS 3
#define N_CUSTOMERS 25
#define SOFA_SIZE 4
#define BARBERSHOP_CAPACITY 20

void *barber_routine(void *args);
void *customer_routine(void *args);

void initialize_mutexes();
void destroy_mutexes();
void initialize_semaphores();
void destroy_semaphores();

pthread_mutex_t door, cash_register;
sem_t sofa, barber;

int customers_count = 0;
queue_t *sofa_queue;

int cash_register_is_occupied = FALSE;

barber_t *barbers[N_BARBERS];
customer_t *customers[N_CUSTOMERS];

int main()
{
    int i;
    pthread_t barbers_threads[N_BARBERS], customers_threads[N_CUSTOMERS];
    
    sofa_queue = createQueue(SOFA_SIZE);

    initialize_mutexes();
    initialize_semaphores();

    for (i = 0; i < N_BARBERS; i++)
    {
        int id = i;
        barbers[i] = make_barber(id);
        if (pthread_create(&barbers_threads[i], NULL, barber_routine, (void *) barbers[i]->id) != 0) {
            perror("Problema na criacao da thread\n");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < N_CUSTOMERS; i++)
    {
        int id = i;
        customers[i] = make_customer(id);
        if (pthread_create(&customers_threads[i], NULL, customer_routine, (void *) customers[i]->id) != 0) {
            perror("Problema na criacao da thread\n");
            exit(EXIT_FAILURE);
        }
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

    destroy_queue(sofa_queue);

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
        pthread_mutex_lock(&cash_register);
        if (!isEmpty(barbers[id]->cash_register_queue) && cash_register_is_occupied == FALSE) {
            printf("Barbeiro #%d foi ao caixa\n", id);

            cash_register_is_occupied = TRUE;
            pthread_mutex_unlock(&cash_register);
            while(!isEmpty(barbers[id]->cash_register_queue)) {
                customer_id = pop(barbers[id]->cash_register_queue);
                customer = customers[customer_id];
                printf("Barbeiro #%d cobrou o cliente #%d\n", id, customer_id);
                sem_post(&(customer->sem));
            }
            pthread_mutex_lock(&cash_register);
            cash_register_is_occupied = FALSE;
            pthread_mutex_unlock(&cash_register);

        } else if (!isEmpty(sofa_queue)) {
            printf("Barbeiro #%d foi cortar cabelo\n", id);
            sem_post(&barber);
            customer_id = pop(sofa_queue);
            pthread_mutex_unlock(&cash_register);
            customer = customers[customer_id];
            sem_post(&(customer->sem));

            // inicia o atendimento
            sleep(1);
            // atendimento finalizado
            // va para a fila de pagamento
            sem_post(&(customer->sem));
            push(barbers[id]->cash_register_queue, get_id(customer));
        } else {
            // Caixa ocupado e sofa vazio, ou esperando os outros barbeiros terminarem
            // para encerrar o dia.
            pthread_mutex_unlock(&cash_register);
            sleep(1);
        }
    }
}

void *customer_routine(void *arg)
{
    int id = *((int *) arg);
    customer_t *customer = customers[id];

    // Cliente chegou na porta
    pthread_mutex_lock(&door);

    if (customers_count == BARBERSHOP_CAPACITY) {
        pthread_mutex_unlock(&door);
        printf("Loja cheia! Cliente #%d foi embora.\n", id);
        pthread_exit((void *) EXIT_SUCCESS);
    }
    customers_count++;
    pthread_mutex_unlock(&door);
    printf("Cliente #%d entrou na loja.\n", id);

    // Esperando abrir lugar no sofa
    sem_wait(&sofa);
    push(sofa_queue, id);
    printf("Cliente #%d sentou no sofa.\n", id);
    // Esperando ter um barbeiro disponivel
    sem_wait(&barber);
    // Esperando ser chamado
    sem_wait(&(customer->sem));
    // Liberou espaço no sofa
    sem_post(&sofa);

    printf("Cliente #%d esta sendo atendido.\n", id);

    // Esperando terminar o atendimento
    sem_wait(&(customer->sem));

    printf("Cliente #%d foi atendido.\n", id);

    // Esperando o pagamento;
    printf("Cliente #%d esta esperando para pagar.\n", id);
    sem_wait(&(customer->sem));

    pthread_mutex_lock(&door);
    customers_count--;
    pthread_mutex_unlock(&door);
    printf("Cliente #%d foi embora.\n", id);

    pthread_exit((void *) EXIT_SUCCESS);
}

void initialize_mutexes() {
    pthread_mutex_init(&door, NULL);
    pthread_mutex_init(&cash_register, NULL);
}

void destroy_mutexes() {
    pthread_mutex_destroy(&door);
    pthread_mutex_destroy(&cash_register);
}

void initialize_semaphores() {
    sem_init(&sofa, 0, SOFA_SIZE);
    sem_init(&barber, 0, N_BARBERS);
}

void destroy_semaphores() {
    sem_destroy(&sofa);
    sem_destroy(&barber);
}