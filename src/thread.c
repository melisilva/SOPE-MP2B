#include "./lib.h"
#include "./utils.h"
#include "./server.h"

#include "stdlib.h"

pthread_mutex_t LOCK_STORAGE;

int store(message_t*res){
    if(pthread_mutex_lock(&LOCK_STORAGE) != 0) {
        return 1;
    }

    push(q,res);

    if (pthread_mutex_unlock(&LOCK_STORAGE) != 0) {
        return 1;
    }
    return 0;
}

int load(message_t *request) {
    if(pthread_mutex_lock(&LOCK_STORAGE) != 0) {
        return 1;
    }

    request = (message_t *)q->front;
    pop(q);

    if (pthread_mutex_unlock(&LOCK_STORAGE) != 0) {
        return 1;
    }
    return 0;
}

void* thread_entry_prod(void *arg){
    message_t recv_message = *(message_t*)arg;

    int res = task(recv_message.tskload);
    recv_message.tskres = res;

    if(!full(q)) { //devendo os threads  Consumidor e Produtores ficarbloqueados quando o armazém estiver, respectivamente, vazio ou cheio
        push(q,&recv_message); //we don't have to lock otherwise? https://stackoverflow.com/questions/31105198/how-to-put-mutex-lock-unlock-for-a-specific-condition-in-if-else-ladder
    }
    else{
         if(store(&recv_message)) {
            return NULL;
        }
    }
   

    return NULL;
}

void* consumer_cycle(time_t end_time){
    // This is just a wrapper function that keeps calling the consumer thread function,
    // it's meant to only be called by the consumer thread.
    while (time(NULL) < end_time){
        thread_entry_cons(NULL);
    }
}

void* thread_entry_cons(void *arg) {
    message_t * request;

    if (!empty(q)) {
        request = (message_t *)q->front;
        pop(q);
    } else {
        if (load(request)) {
            free(request);
            return NULL;
        }
    }

    char *private_fifo_path = NULL;
    int path_size = snprintf(private_fifo_path, 0, "/tmp/%d.%lu",
                                request->pid, request->tid) + 1;  
    
    if (path_size == -1) {
        free(request);
        return NULL;
    }

    private_fifo_path = malloc(path_size);

    if (private_fifo_path == NULL) {
        free(request);
        return NULL;
    }
    
    int fd_private_fifo = 0;


    if (snprintf(private_fifo_path, path_size, "/tmp/%d.%lu",
        request->pid, request->tid) < 0 ) {
        free(private_fifo_path);
        free(request);
        return NULL;
    }

    if ((fd_private_fifo = open(private_fifo_path, O_WRONLY)) == -1) {
        fprintf(stderr, "No public pipe found with given path.\n");
        free(private_fifo_path);
        free(request);
    }

    free(private_fifo_path);
    free(request);
    close(fd_private_fifo);
    return NULL;
}