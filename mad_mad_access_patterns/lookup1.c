/**
 * mad_mad_access_patterns
 * CS 241 - Spring 2022
 */
#include "tree.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/

int main(int argc, char **argv) {
    if(argc <= 2) {
      printArgumentUsage();
      exit(1);
    }

    char* filename = argv[1];
    FILE* file = fopen(filename, "r");
    if(file == NULL) {
      openFail(filename);
      exit(2);
    }

    char* btre = malloc(5);
    memset(btre, 0, 5);
    fread(btre, 1, 4, file);
    if(strcmp(btre, "BTRE")) {
      formatFail(filename);
      free(btre);
      exit(2);
    }
    free(btre);
    
    uint32_t* offset = malloc(sizeof(uint32_t));
    uint32_t* count = malloc(sizeof(uint32_t));
    float* price = malloc(sizeof(float));
    
    for(int i = 2; i < argc; i++) {
      char* word = argv[i];
      int len = strlen(word);
      char* found = malloc(len + 2);
      memset(found, 0, len + 2);
      
      fseek(file, 4 + 3*sizeof(uint32_t) + sizeof(float), SEEK_SET);
      fread(found, 1, len + 1, file);
      fseek(file, 4 + 3*sizeof(uint32_t) + sizeof(float), SEEK_SET);
      int res = strcmp(word, found);
      
      while (res != 0) {
        if(res < 0) {
          fseek(file, -(3*sizeof(uint32_t) + sizeof(float)), SEEK_CUR);
        } else if (res > 0) {
          fseek(file, -(2*sizeof(uint32_t) + sizeof(float)), SEEK_CUR);
        }
        fread(offset, 1, sizeof(uint32_t), file);
        
        if(*offset == 0) {
          break;
        }

        fseek(file, *offset + 3*sizeof(uint32_t) + sizeof(float), SEEK_SET);
        fread(found, 1, len + 1, file);
        fseek(file, *offset + 3*sizeof(uint32_t) + sizeof(float), SEEK_SET);
        res = strcmp(word, found);
      }

      if(res) {
        printNotFound(word);
      } else {
        fseek(file, -(sizeof(uint32_t) + sizeof(float)), SEEK_CUR);
        fread(count, 1, sizeof(uint32_t), file);
        fread(price, 1, sizeof(float), file);
        printFound(word, *count, *price);
      }

      free(found);
    }

    free(offset);
    free(count);
    free(price);
    return 0;
}
