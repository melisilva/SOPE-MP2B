#include "./utils.h"

static char * OPERATIONS[] = {"IWANT", "RECVD", "TSKEX", "TSKDN",
                              "GOTRS", "2LATE", "CLOSD", "GAVUP",
                              "FAILD"};


void message_builder(message_t *message, int i, int t, int res)  {
    message->rid = i;
    message->tskload = t;
    message->pid = getpid();
    message->tid = pthread_self();
    message->tskres = res;
}

int log_operation(message_t *message, oper_t oper) { // time is lu or ld? ld
    if (printf("%ld ; %d ; %d ; %d ; %lu ; %d ; %s\n", time(NULL),
                message->rid, message->tskload, message->pid, message->tid,
                message->tskres, OPERATIONS[oper]) < 0) {
        return 1;
    }

    return 0;
}
