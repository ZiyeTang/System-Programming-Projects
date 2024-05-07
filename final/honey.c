// Start with the following code
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
// author: ziyet3

pthread_mutex_t duck = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t duck2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t duck3 = PTHREAD_MUTEX_INITIALIZER;

void quit (const char *mesg) {
    perror (mesg);
    exit (1);
}

void* handle_client (void *fd_ptr) {
    pthread_detach (pthread_self ()); // pthread_join not needed
    int fd = *(int *) fd_ptr;
    free (fd_ptr); fd_ptr = NULL;
    
    struct sockaddr_in address;
    socklen_t addressLength = sizeof (address);
    
    int ok = getpeername (fd, (struct sockaddr *) &address, &addressLength);

    // TODO-2: inet_ntoa is not thread-safe! So make this code safe using the duck.
    // inet_ntoa may return a pointer to the same memory location
    // TODO-3: Answer below: If no lock was used to protect this critical section
    // what errors/changes would you expect to see in the program’s behavior or output?
    // 
    // Answer: In the implementation of inet_ntoa() the return value as an array of char, is static. 
    // And how it's implemented is just reading data from the argument passed in to the array of character.
    // So, when multiple threads calling this function, they would write to this static char array at the same time.
    // Therefore, without mutex lock, some threads may get the ip of client which is served by other threads.
    pthread_mutex_lock(&duck);
    char *filename = ok == 0 ? inet_ntoa (address.sin_addr) : "unknownip";
    pthread_mutex_unlock(&duck);

    // you do not need to call free on the filename
    printf ("Logging data to file %s\n", filename);
    
    
    
    char buffer[8 * 1024 + 1];
    

    size_t numreadtotal = 0;   
    ssize_t r;

    while(numreadtotal  < 8 * 1024  && (!strstr (buffer, "\r\n\r\n")) &&(r = read(fd, buffer + numreadtotal ,1))) {
        if (r == -1 && errno == EINTR) {
            continue;
        }
        numreadtotal  += r;
    }

    pthread_mutex_lock(&duck2);
    FILE *file = fopen(filename,"a");
    fwrite (buffer, 1, numreadtotal, file);
    fclose(file);
    pthread_mutex_unlock(&duck2);
    
    // time_t, time() and ctime() are great but look out for thread safety.
    // HTTP request and response headers both use \r\n as a line ending.
    char* header = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n";
    write(fd, header, strlen(header));

    pthread_mutex_lock(&duck3);
    time_t seconds = time(NULL);
    char* time = ctime(&seconds);
    write(fd, time, strlen(time));
    pthread_mutex_unlock(&duck3);

    
    // TODO-5 Prevent a race condition when two threads write to the same log file:
    // - Concurrent requests must not have overlapping content in the log file.

    
    shutdown(fd , SHUT_WR);
    close(fd);
    return NULL;
}


int main(int argc, char** argv) {
    
    if (argc<2) {
        exit(1);
    }

    
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

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

    if(listen(sock_fd, 10) != 0) {
        perror("listen fail");
        exit(1);
    }

    while(1) {
        int client_fd = accept(sock_fd, NULL, NULL);
        pthread_t tid;
        pthread_create(&tid, 0, handle_client, (void*) &client_fd);
    }
    
    return 0;
}

