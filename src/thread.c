#include "./thread.h"
#include "./lib.h"
#include "./utils.h"
#include "./server.h"
#include "./queue.h"

#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>


typedef struct args_cancel {
    char *private_fifo_path;
    message_t request_message;
} args_t;


int store(message_t*res) {
    sem_wait(&SEM_EMPTY);
    pthread_mutex_lock(&LOCK_STORAGE);

    queue_push(queue, res);

    pthread_mutex_unlock(&LOCK_STORAGE);
    sem_post(&SEM_FULL);
    return 0;
}


int load(message_t *request) {
    sem_wait(&SEM_FULL);
    pthread_mutex_lock(&LOCK_STORAGE);

    queue_front(queue, request);
    queue_pop(queue);

    pthread_mutex_unlock(&LOCK_STORAGE);
    sem_post(&SEM_EMPTY);
    return 0;
}


void* thread_entry_prod(void *arg){
    message_t recv_message = *(message_t*)arg;
    free((message_t*)arg);
    pid_t client_pid = recv_message.pid;
    pthread_t client_tid = recv_message.tid;
    recv_message.pid = getpid();
    recv_message.tid = pthread_self();


    if (SERVER_CLOSED) {
        recv_message.tskres = -1;
    } else {
        recv_message.tskres = task(recv_message.tskload);

        if (log_operation(&recv_message, TSKEX) != 0) {
            return NULL;
        }
    }

    recv_message.pid = client_pid;
    recv_message.tid = client_tid;

    store(&recv_message);

    return NULL;
}


void* thread_entry_cons() {
    message_t request;
    load(&request);
        

    char *private_fifo_path = NULL;
    int path_size = snprintf(private_fifo_path, 0, "/tmp/%d.%lu",
                                request.pid, request.tid) + 1;  
    
    if (path_size == -1) {
        return NULL;
    }

    private_fifo_path = malloc(path_size);

    if (private_fifo_path == NULL) {
        return NULL;
    }

    if (snprintf(private_fifo_path, path_size, "/tmp/%d.%lu",
        request.pid, request.tid) < 0 ) {
        free(private_fifo_path);
        return NULL;
    }

    request.pid = getpid();
    request.tid = pthread_self();

    int fd_private_fifo = 0;

    if ((fd_private_fifo = open(private_fifo_path, O_WRONLY | O_NONBLOCK)) == -1) {
        log_operation(&request, FAILD);
        perror("Private fifo open error ");
        free(private_fifo_path);
        return NULL;
    }

    int n = write(fd_private_fifo, &request, sizeof(message_t));
    
    if (n < 0) {
        perror("Couldn't write in private FIFO");
        close(fd_private_fifo);
        free(private_fifo_path);
        return NULL;
    } else {
        if (request.tskres == -1) {
            if (log_operation(&request, _2LATE) != 0) {
                close(fd_private_fifo);
                free(private_fifo_path);
                return NULL;
            }
        } else {
            if (log_operation(&request, TSKDN) != 0) {
                close(fd_private_fifo);
                free(private_fifo_path);
                return NULL;
            }
        }
        
    }

    free(private_fifo_path);
    close(fd_private_fifo);
    return NULL;
}


void* consumer_cycle(void* args) {
    // This is just a wrapper function that keeps calling the consumer thread function,
    // it's meant to only be called by the consumer thread.

    while (true) {
        thread_entry_cons();
    }

    return NULL;
}
