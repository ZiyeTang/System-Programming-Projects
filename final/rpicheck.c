// author: ziyet3

#include<stdio.h>
#include<stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

void print_usage() {
    printf("Usage: ./a.out <badrpi directory> <backup directory>\n");
}


// return 1 if : 
// i)  badrpi_file is in backup 
// ii) badrpi_file has a different modification time than the corresponding file in the backup
// else return 0.
// I'm not sure what "corresponding" means, so I would just compare their file names
// If two file names are the same, then they correspond.
int check_backup(char* badrpi_file, char* path, time_t badrpi_file_mtime) {
    DIR* dirp = opendir(path);
    if(dirp == NULL) {
        return 1;
    }

    int res = 1;

    struct dirent* dp; 
    char* newpath;
    struct stat st;

    while((dp=readdir(dirp))!=NULL) {
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }

        newpath = malloc(strlen(path)+strlen(dp->d_name)+2);
        sprintf(newpath, "%s/%s", path, dp->d_name);


        if(stat(newpath, &st) == 0 && S_ISDIR(st.st_mode)) {
            res = res && check_backup(badrpi_file, newpath, badrpi_file_mtime);
        } else if(strcmp(badrpi_file, dp->d_name) == 0) {
            if(difftime(st.st_mtime, badrpi_file_mtime)) {
                return 1;
            } else {
                return 0;
            }
        }
        
        free(newpath);
    }
    closedir(dirp);
    return res;
}



void traverse_badrpi(char* path, char* backup_name) {
    DIR* dirp = opendir(path);
    if(dirp == NULL) {
        return;
    }

    struct dirent* dp; 
    char* newpath;
    char* symbolic = malloc(256);
    
    while((dp=readdir(dirp))!=NULL) {
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }

        newpath = malloc(strlen(path)+strlen(dp->d_name)+2);
        sprintf(newpath, "%s/%s", path, dp->d_name);

        // Check if is symbolic link
        ssize_t len = readlink(newpath, symbolic, 256);
        if(len != -1) {
            symbolic[len] = 0;
            printf("%s:%s\n", newpath, symbolic);
            free(newpath);
            continue;
        }

        // Check if the number of hard link is >1
        struct stat st;
        size_t ret = stat(newpath, &st);
        if(st.st_nlink > 1 && S_ISREG(st.st_mode)) {
            // Print path and nextline the file content
            printf("%s\n",newpath);
            FILE* file = fopen(newpath, "r");
            char* buffer = malloc(sizeof(char));

            size_t total_read = 0;
            while(1) {
                size_t ret = fread(buffer, 1, 1, file);
                if (ret!=1) {
                    break;
                }
                total_read += ret;
                if(total_read == 80 && *buffer!='\n') {
                    printf("\n");
                    total_read = 0;
                } else{
                    if(*buffer<32 && *buffer>126) {
                        printf(".");
                    } else {
                        if(*buffer=='\n') {
                            total_read = 0;
                        }
                        printf("%c", *buffer);
                    }
                }
                
            }

            // Assume that a new line is needed after I print the content of the file
            printf("\n");

            free(newpath);
            continue;
        }
        
        
        if(ret == 0 && S_ISDIR(st.st_mode)) {
            traverse_badrpi(newpath, backup_name);
        } else if(check_backup(dp->d_name, backup_name, st.st_mtime)) {
            printf("%s\n", newpath);
        }
        
        free(newpath);
    }

    closedir(dirp);
    free(symbolic);
}


int main(int argc, char **argv) {

    // check if have incorrect number of argument
    if(argc != 3) {
        print_usage();
        exit(1);
    }

    DIR* badrpi = opendir(argv[1]);
    DIR* backup = opendir(argv[2]);

    // check if the directory names are invalid
    if(badrpi == NULL || backup == NULL) {
        print_usage();
        exit(1);
    }
    closedir(badrpi);
    closedir(backup);

    traverse_badrpi(argv[1], argv[2]);
    
    return 0;
}