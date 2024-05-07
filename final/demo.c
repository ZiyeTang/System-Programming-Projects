#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <string.h>



int main() {

    FILE *file2 = fopen("test.txt","w");
    time_t seconds = time(NULL);
    char* time = ctime(&seconds);
    fwrite(time, 1, strlen(time), file2);

    fclose(file2);
}