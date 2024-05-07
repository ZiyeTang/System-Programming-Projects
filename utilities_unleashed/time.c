/**
 * utilities_unleashed
 * CS 241 - Spring 2022
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "format.h"
#include <string.h>

int main(int argc, char *argv[]) {
    
    pid_t pid = fork();
    if (pid < 0) { // fork failure
        print_fork_failed();
        exit(1);
    } else if (pid > 0) {
        int status;
        struct timespec start, stop;
        clock_gettime(CLOCK_MONOTONIC, &start);
        waitpid(pid, &status, 0);
        clock_gettime(CLOCK_MONOTONIC, &stop);
        double accum = (double) stop.tv_sec - (double) start.tv_sec + ( (double) stop.tv_nsec - (double) start.tv_nsec )/ 1000000000;
        if(!status) {
            display_results(argv, accum);
        }
    } else {
        if(argv[1]==NULL) {
            print_time_usage();
            exit(1);
        }
        //char* inst = malloc(30);
        //strcpy(inst ,"/bin/");
        //strcat(inst,argv[1]);
        execvp(argv[1], argv+1);
        //free(inst);
        print_exec_failed();
        exit(1); // For safety.
    }
    return 0;
}
