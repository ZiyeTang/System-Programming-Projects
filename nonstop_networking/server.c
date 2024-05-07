/**
 * nonstop_networking
 * CS 241 - Spring 2022
 */
#include <stdio.h>
#include "common.h"
#include "format.h"
#include "vector.h"

#define MAX_NUM_EVENTS 100

static char* dir;
static vector* filelist;
static size_t list_size;
static int epollfd;

void handle_sigint (int signal) {
    char* curname;
    char* path;
    for(size_t i = 0; i < vector_size(filelist); i++) {
        curname = vector_get(filelist, i);
        path = malloc(strlen(curname)+strlen(dir)+2);
        memset(path, 0, strlen(curname)+strlen(dir)+2);
        sprintf(path, "%s/%s", dir, curname);
        unlink(path);
        free(path);
    }
    printf("%d\n",rmdir(dir));
    vector_destroy(filelist);
    close(epollfd);
    exit(1);
}

void handle_sigpipe (int signal) {}


char* read_request_header(int fd) {
    char* request = malloc(264);
    memset(request, 0, 264);

    int bytes_read = 0;
    ssize_t r;
    while(bytes_read < 263) {
        r = my_read(fd, request + bytes_read, 1);
        if(r <= 0) {
            print_invalid_response();
            kill(getpid(), SIGINT);
        }
        if(request[bytes_read]==10){
            break;
        }
        bytes_read++;
    }

    if(request[bytes_read]!=10 || strlen(request) < 5) {
        print_invalid_response();
        kill(getpid(), SIGINT);
    }

    if(strncmp(request, "GET ", 4) && strncmp(request, "PUT ", 4) && strncmp(request, "LIST\n", 5) && strncmp(request, "DELETE ", 7)) {
        print_invalid_response();
        kill(getpid(), SIGINT);
    }

    request[bytes_read] = 0;
    return request;

}


void send_response(int fd, char* request) {
    ssize_t r;
    char* filename = NULL;
    
    if(strchr(request, ' ')!=NULL) {
        filename = strchr(request, ' ') + 1;
    } 

    char* path = NULL;
    if(filename != NULL) {
        path = malloc(strlen(filename) + strlen(dir) + 2);
        memset(path, 0 ,strlen(filename) + strlen(dir) + 2);
        sprintf(path, "%s/%s", dir, filename);
    }
    FILE* file = NULL;

    size_t bytes_rw = 0;
    size_t to_rw;
    char* buffer = malloc(4097);    
    memset(buffer, 0, 4097);

    if(strncmp(request, "GET ", 4) == 0) {
        file = fopen(path, "r");
        if(file == NULL) {
            r = my_write(fd, "ERROR\nNo such file\n", 19);
            if(r <=0) {
                perror("send response failed");
                exit(1);
            }
            return;
        }
        

        r = my_write(fd, "OK\n", 3);
        if(r <=0) {
            perror("send response failed");
            exit(1);
        }

        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0 ,SEEK_SET);

        r = my_write(fd, (char*)&size, sizeof(size_t));
        if(r <=0) {
            perror("send response failed");
            exit(1);
        }
        
        while(bytes_rw < size) {
            to_rw = 4096;
            if(size - bytes_rw < to_rw) {
                to_rw = size - bytes_rw;
            }

            fread(buffer, 1, to_rw, file);

            r = my_write(fd, buffer, to_rw);
            if(r <= 0) {
                perror("send response failed");
                exit(1);
            }
            
            bytes_rw += r;
        }
    } else if(strncmp(request, "PUT ", 4) == 0) {
        file = fopen(path, "r");
        if(file == NULL) {
            vector_push_back(filelist, filename);
            if(list_size == 0) {
                list_size = strlen(filename);
            } else {
                list_size += 1 + strlen(filename);
            }
        } else {
            fclose(file);
        }

        file = fopen(path, "w");


        size_t* size = malloc(sizeof(size_t));
        r = my_read(fd, (char*) size, sizeof(size_t));
        if(r <=0) {
            print_invalid_response();
            kill(getpid(), SIGINT);
        }

        while(bytes_rw < *size) {
            to_rw = 4096;
            if(*size - bytes_rw < to_rw) {
                to_rw = *size - bytes_rw;
            }

            r = my_read(fd, buffer, to_rw);
            if(r < 0) {
                perror("read client socket fd fail");
                exit(1);
            }
            if(r == 0) {
                if (bytes_rw < *size) {
                    r = my_write(fd, "ERROR\n", 6);
                    if(r <=0) {
                        perror("send response failed");
                        exit(1);
                    }

                    r = my_write(fd, "Received too little data\n", 25);
                    if(r <=0) {
                        perror("send response failed");
                        exit(1);
                    }
                    free(size);
                    return;
                }
            }

            fwrite(buffer, 1, r, file);
            
            bytes_rw += r;
        }
        free(size);

        r = my_read(fd, buffer, 4096);
        if (r != 0) {
            r = my_write(fd, "ERROR\n", 6);
            if(r <=0) {
                perror("send response failed");
                exit(1);
            }

            r = my_write(fd, "Received too much data\n", 23);
            if(r <=0) {
                perror("send response failed");
                exit(1);
            }      
            return;
        }


        r = my_write(fd, "OK\n", 3);
        if(r <=0) {
            perror("send response failed");
            exit(1);
        }

    } else if(strlen(request) > 7 && strncmp(request, "DELETE ", 7) == 0) {
        r = remove(path);
        if(r == 0) {
            r = my_write(fd, "OK\n", 3);
            if(r <=0) {
                perror("send response failed");
                exit(1);
            }

            if(vector_size(filelist) == 1) {
                list_size = 0;
            } else {
                list_size -= 1 + strlen(filename);
            }

            char* curname;
            for(size_t i = 0; i < vector_size(filelist); i++) {
                curname = vector_get(filelist, i);
                if(strcmp(curname, filename) == 0) {
                    vector_erase(filelist, i);
                    break;
                }
            }
        } else {
            r = my_write(fd, "ERROR\nNo such file\n", 19);
            if(r <=0) {
                perror("send response failed");
                exit(1);
            }
        }
    } else if(strncmp(request, "LIST", 4) == 0) {
        r = my_write(fd, "OK\n", 3);
        if(r <=0) {
            perror("send response failed");
            exit(1);
        }

        r = my_write(fd, (char*) &list_size, sizeof(size_t));
        if(r <=0) {
            perror("send response failed");
            exit(1);
        }
        char* curname;
        for(size_t i = 0; i < vector_size(filelist); i++) {
            curname = vector_get(filelist, i);
            r = my_write(fd, curname, strlen(curname));
            if(r <=0) {
                perror("send response failed");
                exit(1);
            }

            if(i != vector_size(filelist) - 1) {
                r = my_write(fd, "\n", 1);
                if(r <=0) {
                    perror("send response failed");
                    exit(1);
                }
            }
        }

    }
    
    free(buffer);

    if(path!=NULL) {
        free(path);
    }
    if(file!=NULL) {
        fclose(file);
    }
    
}

void handle_client_request(int fd) {
    char* request = read_request_header(fd);
    send_response(fd, request);
    free(request);
    shutdown(fd, SHUT_RDWR);
    close(fd);
}

int main(int argc, char **argv) {
    // good luck!

    signal(SIGINT, handle_sigint);
    signal(SIGPIPE, handle_sigpipe);

    if(argc != 2) {
        print_server_usage();
        exit(1);
    }

    char template[] = "XXXXXX";
    dir = mkdtemp(template);
    print_temp_directory(dir);

    filelist = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    
    list_size = 0;
    
    int sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        perror("get address info fail");
        exit(1);
    }

    
    if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) <0 ) {
        perror("setsockopt fail");
        exit(1);
    }

    if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int)) <0 ) {
        perror("setsockopt fail");
        exit(1);
    }

    if(bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind fail");
        exit(1);
    }

    if(listen(sock_fd, MAX_NUM_EVENTS) != 0) {
        perror("listen fail");
        exit(1);
    }

    epollfd = epoll_create(42);
    struct epoll_event ev = {.events = EPOLLIN, .data.fd = sock_fd};
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sock_fd, &ev);


    while(1) {
        struct epoll_event events_arr[MAX_NUM_EVENTS];
        int num_events = epoll_wait(epollfd, events_arr, MAX_NUM_EVENTS, 1000);
        if(num_events == -1) {
            perror("epoll_wait fail");
            exit(1);
        }

        for(int i = 0; i < num_events; i++) {
            
            int cur_fd = events_arr[i].data.fd;
            int cur_event = events_arr[i].events;

            if(cur_fd == sock_fd) {
                int client_fd = accept(cur_fd, NULL, NULL);
                if(client_fd < 0) {
                    perror("accpet fail");
                    exit(1);
                }

                ev.events  = EPOLLIN;
                ev.data.fd = client_fd; 
                epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &ev);

                if(cur_event & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, cur_fd, NULL);
                    
                }
            } else if(cur_event & EPOLLIN) {
                handle_client_request(cur_fd);
            }
        }
        
    }
}
