#include "./lib.h"
#include "./utils.h"
#include "./server.h"

pthread_mutex_t LOCK_STORAGE;

int store(int*res){
    if(pthread_mutex_lock(&LOCK_STORAGE) != 0) {
        return 1;
    }

    push(q,res);

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
        push(q, &res); //we don't have to lock otherwise? https://stackoverflow.com/questions/31105198/how-to-put-mutex-lock-unlock-for-a-specific-condition-in-if-else-ladder
    }
    else{
         if(store(&res)) {
            return NULL;
        }
    }
   

    return NULL;
}

void* thread_entry_cons(void *arg){
    return NULL;
}