#ifndef SRC_THREADS_H_
#define SRC_THREADS_H_

extern pthread_mutex_t LOCK_STORAGE;

void* thread_entry_prod(void *arg);

void* thread_entry_cons(void *arg);

void *consumer_cycle(time_t end_time);

#endif  // SRC_THREADS_H_