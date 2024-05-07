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
    if (pid < 0) {
        print_fork_failed();
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        if(argc==1) {
            print_env_usage();
            exit(1);
        }

        int i;
        for(i = 1; i < argc; i++) {
            
            if(!strcmp(argv[i],"--")) {
                i++;
                break;
            } else {
                char* equal = strchr(argv[i],'=');
                if(equal == NULL) {
                    print_env_usage();
                    exit(1);
                }
                int idx = equal-argv[i];
                char* varName = malloc(30);
                memcpy( varName, argv[i], idx);
                varName[idx] = '\0';
                
                if(strlen(varName)==0) {
                    print_environment_change_failed();
                    free(varName);
                    exit(1);
                }
                if(*(equal+1)=='%') {
                    char* val = getenv(equal+2);
                    if(val==NULL) {
                        print_environment_change_failed();
                        exit(1);
                    }
                    setenv(varName, val, 1);
                } else {
                    setenv(varName, equal+1, 1);
                }
            }

        }

        if (i == argc) {
            print_env_usage();
            exit(1);
        }

        //char **s = environ;

        /*for (; *s; s++) {
            printf("%s\n", *s);
        }*/

        //char* inst = malloc(30);
        //strcpy(inst ,"/bin/");
        //strcat(inst,argv[i]);
        execvp(argv[i], argv+i);
        //free(inst);
        print_exec_failed();
        exit(1); 
    }
    return 0;
}
