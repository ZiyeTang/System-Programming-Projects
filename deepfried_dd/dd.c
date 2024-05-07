/**
 * deepfried_dd
 * CS 241 - Spring 2022
 */
#include "format.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static size_t full_blocks_in = 0;
static size_t partial_blocks_in = 0;
static size_t full_blocks_out = 0;
static size_t partial_blocks_out = 0;
static int bytes_copied = 0;
static double time_elapsed = 0;
static struct timespec* start = NULL;
static struct timespec* end = NULL;
static char* buffer;

void signal_handler(int sig) {
    
    if(start!=NULL && end!=NULL) {
        clock_gettime(CLOCK_REALTIME, end);
        time_elapsed = ((double)(end->tv_sec) - (double)(start->tv_sec)) + ((double)(end->tv_nsec) - (double)(start->tv_nsec))/1000000000;
    }
    
    print_status_report(full_blocks_in, partial_blocks_in, full_blocks_out, partial_blocks_out, bytes_copied, time_elapsed);

    free(start);
    free(end);
    free(buffer);
    exit(1);
}

int main(int argc, char **argv) {
    signal(SIGUSR1, signal_handler);

    int opt;
    char* inputname = NULL;
    char* outputname = NULL;
    int block_size = 512;
    int nblk_cpy = -1;
    int in_nblk_skip = 0;
    int out_nblk_skip = 0;
    while ((opt = getopt(argc, argv, "i:o:b:c:p:k:")) != -1) {
        switch (opt)
        {
        case 'i':
            inputname = strdup(optarg);
            break;

        case 'o':
            outputname = strdup(optarg);
            break;
        
        case 'b':
            block_size = atoi(optarg);
            
            break;

        case 'c':
            nblk_cpy = atoi(optarg);
            break;

        case 'p':
            in_nblk_skip = atoi(optarg);
            break;

        case 'k':
            out_nblk_skip = atoi(optarg);
            break;

        default:
            break;
        }
    }


    buffer = malloc(block_size);
    start = malloc(sizeof(struct timespec));
    end = malloc(sizeof(struct timespec));

    clock_gettime(CLOCK_REALTIME, start);
    if(inputname == NULL && outputname == NULL) {
        while (nblk_cpy == -1 || bytes_copied < nblk_cpy * block_size) {
            int read_amount = fread(buffer, 1, block_size, stdin);
            if(read_amount == block_size) {
                full_blocks_in++;
            } else if(read_amount!=0) {
                partial_blocks_in++;
            }

            int write_amount = fwrite(buffer, 1, read_amount, stdout);
            if(write_amount == block_size) {
                full_blocks_out++;
            } else if(write_amount!=0) {
                partial_blocks_out++;
            }
            bytes_copied += write_amount;

            if(read_amount != block_size) {
                break;
            }
        }
    } else if (inputname == NULL) {
        FILE* outfile = fopen(outputname, "r+");

        if(outfile == NULL) {
            outfile = fopen(outputname, "w");
            if(outfile == NULL) {
                print_invalid_output(outputname);
                exit(1);
            }
        }
        free(outputname);

        fseek(outfile, out_nblk_skip * block_size, SEEK_SET);

        while (nblk_cpy == -1 || bytes_copied < nblk_cpy * block_size) {
            int read_amount = fread(buffer, 1, block_size, stdin);
            if(read_amount == block_size) {
                full_blocks_in++;
            } else if(read_amount!=0) {
                partial_blocks_in++;
            }

            int write_amount = fwrite(buffer, 1, read_amount, outfile);
            if(write_amount == block_size) {
                full_blocks_out++;
            } else if(write_amount!=0) {
                partial_blocks_out++;
            }

            bytes_copied += write_amount;

            if(read_amount != block_size) {
                break;
            }
        }
        fclose(outfile);

    } else if (outputname == NULL) {
        FILE* infile = fopen(inputname, "r");
        
        if(infile == NULL) {
            print_invalid_input(inputname);
            exit(1);
        }

        free(inputname);

        int total_size;
        if(nblk_cpy == -1) {
            fseek(infile, 0, SEEK_END);
            total_size = ftell(infile);
            fseek(infile, 0, SEEK_SET);
        } else {
            total_size = nblk_cpy * block_size;
        }

        fseek(infile, in_nblk_skip * block_size, SEEK_SET);

        while (bytes_copied < total_size) {
            int read_amount = fread(buffer, 1, block_size, infile);
            if(read_amount == block_size) {
                full_blocks_in++;
            } else if(read_amount!=0) {
                partial_blocks_in++;
            }

            int write_amount = fwrite(buffer, 1, read_amount, stdout);
            if(write_amount == block_size) {
                full_blocks_out++;
            } else if(write_amount!=0) {
                partial_blocks_out++;
            }

            bytes_copied += write_amount;

            if(read_amount != block_size) {
                break;
            }
        }
        fclose(infile);

    } else {
        FILE* infile = fopen(inputname, "r");
        FILE* outfile = fopen(outputname, "r+");

        if(infile == NULL) {
            print_invalid_input(inputname);
            exit(1);
        }

        if(outfile == NULL) {
            outfile = fopen(outputname, "w");
            if(outfile == NULL) {
                print_invalid_output(outputname);
                exit(1);
            }
        }

        free(inputname);
        free(outputname);

        int total_size;
        if(nblk_cpy == -1) {
            fseek(infile, 0, SEEK_END);
            total_size = ftell(infile);
            fseek(infile, 0, SEEK_SET);
        } else {
            total_size = nblk_cpy * block_size;
        }

        fseek(infile, in_nblk_skip * block_size, SEEK_SET);
        fseek(outfile, out_nblk_skip * block_size, SEEK_SET);

        while (bytes_copied < total_size) {
            int read_amount = fread(buffer, 1, block_size, infile);
            if(read_amount == block_size) {
                full_blocks_in++;
            } else if(read_amount!=0) {
                partial_blocks_in++;
            }

            int write_amount = fwrite(buffer, 1, read_amount, outfile);
            if(write_amount == block_size) {
                full_blocks_out++;
            } else if(write_amount!=0) {
                partial_blocks_out++;
            }

            bytes_copied += write_amount;

            if(read_amount != block_size) {
                break;
            }
        }

        fclose(infile);
        fclose(outfile);
    }

    clock_gettime(CLOCK_REALTIME, end);
    time_elapsed = ((double)(end->tv_sec) - (double)(start->tv_sec)) + ((double)(end->tv_nsec) - (double)(start->tv_nsec))/1000000000;
    print_status_report(full_blocks_in, partial_blocks_in, full_blocks_out, partial_blocks_out, bytes_copied, time_elapsed);

    free(start);
    free(end);
    free(buffer);
    return 0;
}