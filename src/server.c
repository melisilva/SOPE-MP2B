#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include "./utils.h"
#include "./lib.h"


int main_cycle(time_t end_time, int fd_public_fifo) {
    size_t size_tids = 1000;
    pthread_t *tids = malloc(size_tids * sizeof(pthread_t));
    message_t message_received;

    if (tids == NULL){
        return 1;
    }

    size_t i = 0;
    while (time(NULL) < end_time ) {
        // Gets 
       if(read(fd_public_fifo, &message_received, sizeof(message_t))<0){
           perror("Couldn't read public FIFO");
           return 1;
       }
       
       // Message received.
       message_builder(&message_received, message_received.rid, message_received.tskload, message_received.tskres);
       log_operation(&message_received, RECVD);
        

       // Acusa ter recebido a mensagem.
       log_operation(&message_received, TSKDN);



    }



    return 0;
}


int input_check(int argc, char *argv[], int *nsecs, int *bufsz,int *fd_public_fifo) {
    //s -t nsecs -l bufsz fifoname
    if (argc != 6) {
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

    if (strcmp(argv[3], "-s")) {
        // id argv[3] != "-s"
        fprintf(stderr, "Expected -s parameter not found.\n");
        return 1;
    }

    *bufsz = strtol(argv[4], &end, 10);
    if (argv[4] == end) {
        fprintf(stderr, "Invalid storage size.\n");
        return 1;
    }

    if (mkfifo(argv[5], 0666) != 0) {
        fprintf(stderr, "Not possible to make public fifo.\n");
        return 1;
    }

    if ((*fd_public_fifo = open(argv[5], O_RDONLY)) == -1) {
         fprintf(stderr, "Not possible to open public fifo.\n");
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {

    // just to check that's working
    for (int i = 0; i < 100; i++)
        printf("res: %d\n", task(9));

    /*
    time_t start_time = time(NULL);
    

    //RAND_R_SEED = start_time;
    int nsecs;
    int bufsz;
    int fd_public_fifo;

    if (input_check(argc, argv, &nsecs, &bufsz,&fd_public_fifo) != 0) {
        return 1;
    }

    
    time_t end_time = start_time + nsecs;
    if(main_cycle(end_time, fd_public_fifo)){
        close(fd_public_fifo);
        return 1;
    }
    
    
    

    close(fd_public_fifo);
    //printf("we're closed\n");
*/
    return 0;
}