#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>

#include "./utils.h"
#include "./lib.h"
#include "./thread.h"
#include "./queue.h"

#define PUBLIC_PERMS 0666
#define DEFAULT_BUFFER_SIZE 1 // TODO what is the defaut value?

bool SERVER_CLOSED = false;
queue_t *queue = NULL;


int main_cycle(time_t end_time, int fd_public_fifo) {
    int ret = 0;
    message_t message_received;
    size_t size_tids = 1000;
    pthread_t *tids = malloc(size_tids * sizeof(pthread_t));
    pthread_t ctid;
    size_t i = 0;

    if (tids == NULL)
        return 1;

    //Create consumer thread?-->It's a single thread but needs to be work constantly
    while (pthread_create(&ctid, NULL, consumer_cycle, (void*)&end_time) != 0);

    while (time(NULL) < end_time + OVERTIME_SECONDS_2LATE) { //server only stops when time runs out
        //get info in public fifo
        //while EOF
        int r_res;
        bool _break = false;
        while ((r_res = read(fd_public_fifo, &message_received, sizeof(message_t))) <= 0) {
            if (time(NULL) >= end_time + OVERTIME_SECONDS_2LATE) { // while loop condition
                _break = true;
                break;
            }

            if (r_res == -1) {
                perror("Erro main read ");
                ret = 1;
                _break = true;
                break;
            }
        }

        if (_break)
            break;

        message_t log_message;
        message_builder(&log_message, message_received.rid, message_received.tskload, message_received.tskres);
        if (log_operation(&log_message, RECVD) != 0) {
           free(tids);
           return 1;
        }

        if (i == size_tids) {
            size_tids += 1000;
            pthread_t *new_tids = realloc(tids, size_tids * sizeof(pthread_t));

            if (new_tids == NULL) {
                ret = 1;
                break; // could not allocate more bytes
            } else {
                tids = new_tids;
            }
        }

        while (pthread_create(&tids[i], NULL, thread_entry_prod, (void*)&message_received) != 0); //Produtores-->various

        // if 2LATE
        SERVER_CLOSED = SERVER_CLOSED || (time(NULL) > end_time);
        i++;
    }

    for (size_t j = 0; j < i; j++) {
        pthread_join(tids[j], NULL);
    }

    pthread_cancel(ctid);
    pthread_join(ctid, NULL);

    free(tids);
    return ret;
}


int input_check(int argc, char *argv[], int *nsecs, size_t *bufsz,int *fd_public_fifo) {
    //s -t nsecs -l bufsz fifoname
    //s -t nsecs fifoname
    if (argc != 6 && argc != 4) {
        fprintf(stderr, "Invalid number of arguments.\n");
        return 1;
    }

    if (strcmp(argv[1], "-t")) {
        // id argv[1] != "-t"
        fprintf(stderr, "Expected -t parameter not found.\n");
        return 1;
    }

    char *end;
    *nsecs = strtol(argv[2], &end, 10);
    if (argv[2] == end) {
        fprintf(stderr, "Invalid number of seconds.\n");
        return 1;
    }

    if (argc == 6) {
        if (strcmp(argv[3], "-l")) {
            // id argv[3] != "-l"
            fprintf(stderr, "Expected -l parameter not found.\n");
            return 1;
        }

        *bufsz = strtol(argv[4], &end, 10);
        if (argv[4] == end) {
            fprintf(stderr, "Invalid storage size.\n");
            return 1;
        }
    } else {
        *bufsz = DEFAULT_BUFFER_SIZE;
    }

    if (mkfifo(argv[3 + 2*(argc == 6)], PUBLIC_PERMS) != 0) {
        perror("Failed creating public fifo ");
    }

    if ((*fd_public_fifo = open(argv[3 + 2*(argc == 6)], O_RDONLY)) == -1) {
        perror("");
        //fprintf(stderr, "Not possible to open public fifo.\n");
        unlink(argv[3 + 2*(argc == 6)]);
        return 1;
    }

    return 0;
}


int init_mutex_and_semaphores(size_t semaphore_size) {
    if (pthread_mutex_init(&LOCK_STORAGE, NULL) != 0) {
        perror("");
        return 1;
    }

    if (sem_init(&SEM_FULL, 0, 0) != 0) {
        perror("");
        return 1;
    }

    if (sem_init(&SEM_EMPTY, 0, semaphore_size) != 0) {
        perror("");
        return 1;
    }

    return 0;
}


void destroy_mutex_and_semaphores() {
    // not returning on error so that every destroyer is called
    if (pthread_mutex_destroy(&LOCK_STORAGE) != 0)
        perror("");

    if (sem_destroy(&SEM_FULL) != 0)
        perror("");

    if (sem_destroy(&SEM_EMPTY) != 0)
        perror("");
}


int main(int argc, char *argv[]) {
    time_t start_time = time(NULL);
    int nsecs;
    size_t bufsz;
    int fd_public_fifo;
    queue = new_queue(sizeof(message_t));

    if (input_check(argc, argv, &nsecs, &bufsz, &fd_public_fifo) != 0) {
        delete_queue(queue);
        return 1;
    }

    if (init_mutex_and_semaphores(bufsz) != 0) {
        delete_queue(queue);
        close(fd_public_fifo);
        unlink(argv[3 + 2*(argc == 6)]);
        return 1;
    }
    
    time_t end_time = start_time + nsecs;
    if (main_cycle(end_time, fd_public_fifo) != 0) {
        delete_queue(queue);
        close(fd_public_fifo);
        unlink(argv[3 + 2*(argc == 6)]);
        return 1;
    }

    delete_queue(queue);
    destroy_mutex_and_semaphores();
    close(fd_public_fifo);
    unlink(argv[3 + 2*(argc == 6)]);

    return 0;
}
