/**
 * nonstop_networking
 * CS 241 - Spring 2022
 */
#include "common.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


ssize_t my_write(int fd, const char* buffer, size_t size) {
    size_t bytes_written = 0;
    ssize_t r;
    while(bytes_written < size && (r = write(fd, buffer + bytes_written, size-bytes_written))) {

        if (r == -1 && errno == EINTR) {
            continue;
        }
        if(r == -1) {
            return -1;
        }

        bytes_written += r;
    }
    return bytes_written;
}


ssize_t my_read(int fd, char* buffer, size_t size) {
    size_t byter_read = 0;
    ssize_t r;
    while(byter_read < size && (r = read(fd, buffer + byter_read, size-byter_read))) {

        if (r == -1 && errno == EINTR) {
            continue;
        }

        if(r == -1) {
            return -1;
        }

        byter_read += r;
    }
    return byter_read;
}