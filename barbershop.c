#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "queue.c"

#define BARBERS 3
#define CUSTOMERS 20

void * barberThread(void * args);
void * customerThread(void * args);

sem_t sofa, customer1, customer2, payment, receipt;
pthread_mutex_t mutex1, mutex2;

int customers = 0, counter = 0;
struct Queue *queue_1, *queue_2;

int main() {
    int i;
    pthread_t barbers[BARBERS], customers[CUSTOMERS];
    unsigned long barbers_id[BARBERS], customers_id[CUSTOMERS];
    
    queue_1 = createQueue(CUSTOMERS);
    queue_2 = createQueue(BARBERS);

    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);

    sem_init(&sofa, 0, 4);
    sem_init(&customer1, 0, 0);
    sem_init(&customer2, 0, 0);
    sem_init(&payment, 0, 0);
    sem_init(&receipt, 0, 0);

    for(i = 0; i < BARBERS; i++) {
        barbers_id[i] = (i+1)*100;
        if(pthread_create(&barbers[i], NULL, barberThread, (void*)&barbers_id[i]) != 0) {
            perror("Problema na criacao da thread");
            exit(EXIT_FAILURE);
        }
    }

    for(i = 0; i < CUSTOMERS; i++) {
        customers_id[i] = i+1;
        if(pthread_create(&customers[i], NULL, customerThread, (void*)&customers_id[i]) != 0) {
            perror("Problema na criacao da thread");
            exit(EXIT_FAILURE);
        }
    }

    for(i = 0; i < CUSTOMERS; i++) {
        if(pthread_join(customers[i], NULL) != 0) {
            perror("Problema no join");
            exit(EXIT_FAILURE);
        }
    }

    sleep(1);
    printf("Todos os clientes foram atendidos");

    for(i = 0; i < BARBERS; i++) {
        printf("--%lu encerrou o seu expediente\n", barbers_id[i]);
        pthread_cancel(barbers[i]);
    }


    sem_destroy(&sofa);
    sem_destroy(&customer1);
    sem_destroy(&customer2);
    sem_destroy(&payment);
    sem_destroy(&receipt);

    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);

    return 0;
}

void * barberThread(void * args) {
    unsigned long id = *((unsigned long*) args);
    while(1) {
        printf("--%lu pronto\n", id);
        sem_t *sem1, *sem2;
        sem_wait(&customer1);

        pthread_mutex_lock(&mutex1);
            sem1 = pop(queue_1);
            sem_post(sem1);
            sem_wait(sem1);
        pthread_mutex_unlock(&mutex1);
        sem_post(sem1);

        sem_wait(&customer2);
        pthread_mutex_lock(&mutex2);
            sem2 = pop(queue_2);
            sem_post(sem2);
            printf("--%lu trouxe cliente\n", id);
        pthread_mutex_unlock(&mutex2);

        printf("--%lu cortou\n", id);
        // Corta cabelo
        sleep(2);

        sem_wait(&payment);
        sem_post(&receipt);
        printf("--%lu deu recibo\n", id);
    }
}

void * customerThread(void * args) {
    unsigned long id = *((unsigned long*) args);
    sem_t sem1, sem2;
    sem_init(&sem1, 0, 0);
    sem_init(&sem2, 0, 0);

    pthread_mutex_lock(&mutex1);
        printf("%lu ENTROU\n", id);
        if(customers == 20) {
            pthread_mutex_unlock(&mutex1);
            printf("%lu sifude loja cheia\n", id);
            return 1;
        }
        customers++;
        push(queue_1, &sem1);
    pthread_mutex_unlock(&mutex1);
    
    sem_post(&customer1);

    sem_wait(&sem1);

    sem_wait(&sofa);
        printf("%lu SENTOU\n", id);
        sem_post(&sem1);
        pthread_mutex_lock(&mutex2);
            push(queue_2, &sem2);
        pthread_mutex_unlock(&mutex2);
        sem_post(&customer2);
        sem_wait(&sem2);
    sem_post(&sofa);

    pthread_mutex_lock(&mutex1);
        sem_post(&payment);
        printf("%lu PAGOU\n", id);
        sem_wait(&receipt);
        printf("%lu RECEBEU\n", id);
        customers--;
    pthread_mutex_unlock(&mutex1);
    return 0;
}
