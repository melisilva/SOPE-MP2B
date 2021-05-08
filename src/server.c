#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>

#include "./utils.h"
#include "./lib.h"
#include "./thread.h"
#include "./queue.h"

#define PUBLIC_PERMS 0666
#define BUFFER_QUEUE 100
#define DEFAULT_BUFFER_SIZE 1 // TODO what is the defaut value?
queue_t * q;

int main_cycle(time_t end_time, int fd_public_fifo) {
    message_t message_received;
    size_t size_tids = 1000;
    size_t i = 0;
    pthread_t *tids = malloc(size_tids * sizeof(pthread_t));
    pthread_t *ctid = malloc(sizeof(pthread_t));

    // Create consumer thread?-->It's a single thread but needs to be work constantly
    // Here's an attempt.
    pthread_create(&ctid, NULL, consumer_cycle, (time_t*) end_time);

    while (time(NULL) < end_time ) { // Server only stops when time runs out
        // Get info in queue
       if(read(fd_public_fifo, &message_received, sizeof(message_t))<0){
           perror("Couldn't read public FIFO");
           return 1;
       }
       message_builder(&message_received, message_received.rid, message_received.tskload, message_received.tskres);
       log_operation(&message_received, RECVD);
       pthread_create(&tids[i], NULL, thread_entry_prod, (void*)&message_received); // Various Producers
       i++;
    }
    return 0;
}


int input_check(int argc, char *argv[], int *nsecs, int *bufsz,int *fd_public_fifo) {
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

    int cod;
    if ((cod = mkfifo(argv[3 + 2*(argc == 6)], PUBLIC_PERMS)) != 0) {
        if (errno != EEXIST) {
            // TODO what to do if fifo with this name already exists?
            // run normally?
            perror("");
            //fprintf(stderr, "Not possible to make public fifo.\n");
            return 1;
        }
    }

    if ((*fd_public_fifo = open(argv[3 + 2*(argc == 6)], O_RDONLY)) == -1) {
        perror("");
        //fprintf(stderr, "Not possible to open public fifo.\n");
        unlink(argv[3 + 2*(argc == 6)]);
        return 1;
    }

    return 0;
}

int init_mutexs() {
    if (pthread_mutex_init(&LOCK_STORAGE, NULL) != 0) {
        perror("");
        return 1;
    }
    return 0;
}

int destroy_mutexs(){
    if (pthread_mutex_destroy(&LOCK_STORAGE) != 0) {
        perror("");
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    time_t start_time = time(NULL);
    
    //RAND_R_SEED = start_time;
    int nsecs;
    int bufsz;
    int fd_public_fifo;

    fprintf(stderr, "main before input check\n");

    if (input_check(argc, argv, &nsecs, &bufsz, &fd_public_fifo) != 0) {
        return 1;
    }

    if (init_mutexs() != 0) {
        close(fd_public_fifo);
        unlink(argv[3 + 2*(argc == 6)]);
        return 1;
    }

    initQueue(q, BUFFER_QUEUE);

    fprintf(stderr, "main before main loop\n");
    
    time_t end_time = start_time + nsecs;
    if (main_cycle(end_time, fd_public_fifo)) {
        close(fd_public_fifo);
        unlink(argv[3 + 2*(argc == 6)]);
        return 1;
    }

    fprintf(stderr, "main after main loop\n");
    
    if (destroy_mutexs() != 0) {
        close(fd_public_fifo);
        unlink(argv[3 + 2*(argc == 6)]);
        return 1;
    }

    close(fd_public_fifo);
    unlink(argv[3 + 2*(argc == 6)]);
    fprintf(stderr, "we're closed\n");

    return 0;
}
