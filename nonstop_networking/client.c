/**
 * nonstop_networking
 * CS 241 - Spring 2022
 */
#include "format.h"
#include "common.h"

char **parse_args(int argc, char **argv);
verb check_args(char **args);


void send_request(char** args, verb command, int sock_fd) {
    size_t len;
    char* message;
    if(command == GET || command == DELETE || command == PUT) {
        len = 3 + strlen(args[2]) + strlen(args[3]);
        message = malloc(len);
        memset(message, 0, len);
        sprintf(message, "%s %s\n", args[2], args[3]);

    } else {
        len = 2 + strlen(args[2]);
        message = malloc(len);
        memset(message, 0, len);
        sprintf(message, "%s\n", args[2]);
    }
    
    ssize_t r = my_write(sock_fd, message, strlen(message)); 
    if(r <= 0) {
        perror("send request failed");
        exit(1);
    }

    free(message);

    if(command == PUT) {
        FILE* localfile= fopen(args[4], "r");

        if(localfile == NULL) {
            perror("open local file fail");
            exit(1);
        }

        fseek(localfile, 0, SEEK_END);
        size_t size = ftell(localfile);
        fseek(localfile, 0, SEEK_SET);

        r = my_write(sock_fd, (char*)&size, sizeof(size_t));
        if(r <=0) {
            perror("send request failed");
            exit(1);
        }


        size_t bytes_rw = 0;
        size_t to_rw;
        char* buffer = malloc(4097);
        memset(buffer, 0, 4097);
        
        while(bytes_rw < size) {
            to_rw = 4096;
            if(size - bytes_rw < to_rw) {
                to_rw = size - bytes_rw;
            }

            fread(buffer, 1, to_rw, localfile);

            r = my_write(sock_fd, buffer, to_rw);
            if(r < 0) {
                perror("send request failed");
                exit(1);
            }
            if(r == 0) {
                print_connection_closed();
                exit(1);
            }
            
            bytes_rw += r;
        }

        free(buffer);
        fclose(localfile);
    }


    if(shutdown(sock_fd, SHUT_WR) != 0) {
        perror("shutdown fail");
    }
}



void handle_response(char** args, verb command, int sock_fd) {
    
    char* response = malloc(7);
    memset(response, 0, 7);
    int bytes_read = 0;
    ssize_t r;
    while(bytes_read<6) {
        r = my_read(sock_fd, response + bytes_read, 1);
        if(r <= 0) {
            print_invalid_response();
            exit(1);
        }
        if(response[bytes_read]==10){
            break;
        }
        bytes_read++;
    }

    if(strcmp(response, "OK\n") != 0) {
        if(strcmp(response, "ERROR\n") != 0) {
            print_invalid_response();
            exit(1);
        } else {
            
            char* err = malloc(1025);
            memset(err, 0, 1025);
            bytes_read = 0;
            while(bytes_read<1024) {
                r = my_read(sock_fd, err + bytes_read, 1);
                if(r <= 0) {
                    print_invalid_response();
                    exit(1);
                }
                if(err[bytes_read]==10){
                    break;
                }
                bytes_read++;
            }

            if(*(err + bytes_read) != '\n') {
                print_invalid_response();
                free(err);
                exit(1);
            }

            *(err + bytes_read)='\0';

            print_error_message(err);
            free(err);
            return;
        }
    }
    free(response);

    if(command == PUT || command == DELETE) {
        print_success();
        return;
    }

    
    

    size_t* specified_size = malloc(sizeof(size_t));
    r = my_read(sock_fd, (char*) specified_size, sizeof(size_t));
    if(r <=0) {
        print_invalid_response();
        exit(1);
    }
    size_t actual_size = *specified_size;
    free(specified_size);
    
    size_t bytes_rw = 0;
    size_t to_rw;
    char* buffer = malloc(4097);    
    memset(buffer, 0, 4097);
    if (command == GET) {
        FILE* localfile= fopen(args[4], "w");
        
        while(bytes_rw < actual_size) {
            to_rw = 4096;
            if(actual_size - bytes_rw < to_rw) {
                to_rw = actual_size - bytes_rw;
            }

            r = my_read(sock_fd, buffer, to_rw);
            if(r <0) {
                print_invalid_response();
                exit(1);
            }
            if(r == 0) {
                if (bytes_rw < actual_size) {
                    print_too_little_data();
                }
                break;
            }

            fwrite(buffer, 1, r, localfile);
            
            bytes_rw += r;
        }

        r = my_read(sock_fd, buffer, 4096);
        if (r != 0) {
            print_received_too_much_data();
        }
        
        fclose(localfile);

    } else {
        
        while(bytes_rw < actual_size) {
            to_rw = 4096;
            if(actual_size - bytes_rw < to_rw) {
                to_rw = actual_size - bytes_rw;
            }

            r = my_read(sock_fd, buffer, to_rw);
            if(r <0) {
                print_invalid_response();
                exit(1);
            }
            if(r == 0) {
                if (bytes_rw < actual_size) {
                    print_too_little_data();
                }
                exit(1);
            }

            r = my_write(1, buffer, r);
            if(r <=0) {
                perror("copy remote file fail");
                exit(1);
            }
            
            bytes_rw += r;
        }
        printf("\n");
        r = my_read(sock_fd, buffer, 4096);
        if (r != 0) {
            print_received_too_much_data();
        }
        
    }
    free(buffer);
}



int main(int argc, char **argv) {
    // Good luck!
    char** args = parse_args(argc, argv);
    verb command = check_args(args);


    struct addrinfo hints, *result;

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int s = getaddrinfo(args[0], args[1], &hints, &result);
    if (s != 0) {
        perror("get address info fail");
        exit(1);
    }

    int sock_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock_fd == -1) {
        perror("socket fail");
        exit(1);
    }

    int connect_result = connect(sock_fd, result->ai_addr, result->ai_addrlen);
    if (connect_result == -1) {
        perror("connect fail");
        exit(1);
    }

    freeaddrinfo(result);

    send_request(args, command, sock_fd);
    handle_response(args, command, sock_fd);
    
    close(sock_fd);
    free(args);
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}