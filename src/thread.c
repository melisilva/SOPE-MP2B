#include "./lib.h"
#include "./utils.h"

void* thread_entry(void *arg){
    message_t recv_message = *(message_t*)arg;

    int res = task(recv_message.tskload);
    recv_message.tskres = res;

}