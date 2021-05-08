#include "./lib.h"
#include "./utils.h"
#include "./server.h"

#include <stdlib.h>

pthread_mutex_t LOCK_STORAGE;

int store(message_t*res) {
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

    recv_message.pid = getpid();
    recv_message.tid = pthread_self();

    if(log_operation(&recv_message, TSKEX) != 0) {
        return NULL;
    }

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

void* thread_entry_cons() {
    message_t * request;

    if(!empty(q)) {
        request = (message_t *)q->front;
        pop(q);
    }
    else {
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

    request->pid = getpid();
    request->tid = pthread_self();

    if((fd_private_fifo = open(private_fifo_path, O_WRONLY)) == -1) {
        //fprintf(stderr, "No public pipe found with given path.\n");
        if(log_operation(request,FAILD) != 0) {
            free(private_fifo_path);
            free(request);
            return NULL;
        }
        free(private_fifo_path);
        free(request);
        return NULL;
    }

    int n = write(fd_private_fifo, request, sizeof(message_t));

    if (n < 0) {
        perror("Couldn't write in private FIFO");
        close(fd_private_fifo);
        free(private_fifo_path);
        free(request);
        return NULL;
    }
    else{
        if(log_operation(request, TSKDN) != 0){
            close(fd_private_fifo);
            free(private_fifo_path);
            free(request);
            return NULL;
        }
    }



    free(private_fifo_path);
    free(request);
    close(fd_private_fifo);
    return NULL;
}

void* consumer_cycle(void* args){
    // This is just a wrapper function that keeps calling the consumer thread function,
    // it's meant to only be called by the consumer thread.

    int end_time = *(time_t*) args;
    while (time(NULL) < end_time) {
        thread_entry_cons();
    }

    return NULL;
}