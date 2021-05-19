#ifndef SRC_THREADS_H_
#define SRC_THREADS_H_

#include <pthread.h>
#include <semaphore.h>


pthread_mutex_t LOCK_STORAGE;
sem_t SEM_FULL;
sem_t SEM_EMPTY;

void* thread_entry_prod(void *arg);

void* thread_entry_cons();

void *consumer_cycle(void *arg);

#endif  // SRC_THREADS_H_
