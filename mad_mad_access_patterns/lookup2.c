/**
 * mad_mad_access_patterns
 * CS 241 - Spring 2022
 */
#include "tree.h"
#include "utils.h"
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/

int main(int argc, char **argv) {
  if(argc <= 2) {
    printArgumentUsage();
    exit(1);
  }

  char* filename = argv[1];
  int fd = open(filename, O_RDONLY);
  if(fd == -1) {
    openFail(filename);
    exit(2);
  }

  struct stat sb;
  fstat(fd, &sb);
  int length =(int)sb.st_size;

  char* filecontent = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
  
  if(strncmp(filecontent, "BTRE", 4)){
		formatFail(filename);
		exit(2);
	}

  uint32_t* offset = malloc(sizeof(uint32_t));
  uint32_t* count = malloc(sizeof(uint32_t));
  float* price = malloc(sizeof(float));

  for(int i = 2; i < argc; i++) {
    char* word = argv[i];
    int len = strlen(word);
    char* found = malloc(len + 2);
    memset(found, 0, len + 2);

    void* temp = filecontent + 4 + 3*sizeof(uint32_t) + sizeof(float);
    memcpy(found, temp, len + 1);

    int res = strcmp(word, found);

    while(res) {
      if(res < 0) {
        temp -= 3*sizeof(uint32_t) + sizeof(float);
      } else if (res > 0) {
        temp -= 2*sizeof(uint32_t) + sizeof(float);
      }
      memcpy(offset, temp, sizeof(uint32_t));

      if(*offset == 0) {
        break;
      }

      temp = filecontent + *offset + 3*sizeof(uint32_t) + sizeof(float);
      memcpy(found, temp, len + 1);
      res = strcmp(word, found);
    }

    if(res) {
      printNotFound(word);
    } else {
      temp -= sizeof(uint32_t) + sizeof(float);
      memcpy(count, temp, sizeof(uint32_t));
      temp += sizeof(uint32_t);
      memcpy(price, temp, sizeof(float));
      printFound(word, *count, *price);
    }

    free(found);
  }

  free(offset);
  free(count);
  free(price);
  return 0;

}
