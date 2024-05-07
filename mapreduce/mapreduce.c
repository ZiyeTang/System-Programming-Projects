/**
 * mapreduce
 * CS 241 - Spring 2022
 */
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    // Create an input pipe for each mapper.
    int num_map = atoi(argv[5]);

    int map_fds[2 * num_map];
    for(int i=0; i<num_map; i++) {
        pipe(map_fds + 2*i);
        descriptors_add(map_fds[2*i]);
        descriptors_add(map_fds[2*i+1]);
    }
    

    // Create one input pipe for the reducer.
    int reduce_fds[2];
    pipe(reduce_fds);
    descriptors_add(reduce_fds[0]);
    descriptors_add(reduce_fds[1]);

    // Open the output file.
    int output_fd = open(argv[2],  O_CREAT|O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR);

    // Start a splitter process for each mapper.
    pid_t pid1[num_map]; 
    for(int i=0; i<num_map; i++) {
        pid1[i] = fork();
        if (pid1[i] < 0) { // fork failure
            exit(1);
        } else if (pid1[i] > 0) {

        } else {
            char index[5];
			sprintf(index, "%d", i);
            dup2(map_fds[2*i + 1], 1);
            descriptors_closeall();
            execlp("./splitter", "./splitter",argv[1], argv[5], index, NULL);
            exit(1); // For safety.
        }
    }
    
    // Start all the mapper processes.
    pid_t pid2[num_map]; 
    for (int i=0; i<num_map; i++) {
        pid2[i] = fork();
        if (pid2[i] < 0) { // fork failure
            exit(1);
        } else if (pid2[i] > 0) {

        } else {
            dup2(map_fds[2*i], 0);
            dup2(reduce_fds[1], 1);
            descriptors_closeall();

            execl(argv[3], argv[3], NULL);
            exit(1); // For safety.
        }
    }
    
    // Start the reducer process.
    pid_t pid3 = fork();
    if (pid3 < 0) { // fork failure
        exit(1);
    } else if (pid3 > 0) {
        descriptors_closeall();
        close(output_fd);
        int status;
		waitpid(pid3, &status, 0);
    } else {
        dup2(reduce_fds[0],0);
        dup2(output_fd,1);
  
        descriptors_closeall();
        execl(argv[4], argv[4], NULL);
        exit(1); // For safety.
    }

    // Wait for the reducer to finish.
    int status1[num_map];
    int status2[num_map];
    for(int i=0; i<num_map; i++) {
        waitpid(pid1[i], status1+i, 0);
        waitpid(pid2[i], status2+i, 0);
    }

    // Print nonzero subprocess exit codes.
    for(int i=0; i<num_map; i++) {
        print_nonzero_exit_status(argv[3], status1[i]);
    }
    for(int i=0; i<num_map; i++) {
        print_nonzero_exit_status(argv[4], status2[i]);
    }
    
    // Count the number of lines in the output file.
    print_num_lines(argv[2]);
    descriptors_destroy();
    return 0;
}
