// Wrapper para semáforos
#include <semaphore.h>

sem_t * make_semaphore(unsigned int value) {
    sem_t * sem;
    sem_init(sem, 0, value);
    return sem;
}

void destroy_semaphore(sem_t *sem) {
    if (sem_destroy(sem) == 1) {
        printf("Falha ao destruir semáforo.");
    }
}