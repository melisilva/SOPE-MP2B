#ifndef _UTILS_H
#define _UTILS_H

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>


typedef enum oper {
    IWANT,
    RECVD,
    TSKEX,
    TSKDN,
    GOTRS,
    _2LATE,
    CLOSD,
    GAVUP,
    FAILD
} oper_t;

typedef struct message {
    int rid;  // request id---> i
    pid_t pid;
    pthread_t tid;
    int tskload;  // task load--->t
    int tskres;  // task result-->res
} message_t;


void message_builder(message_t *message, int i, int t, int res);

int log_operation(message_t *message, oper_t oper);

#endif  // _UTILS_H
