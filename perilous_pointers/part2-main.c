/**
 * perilous_pointers
 * CS 241 - Spring 2022
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);

    int val1 = 132;
    second_step(&val1);

    int num = 8942;
    int *val2 = &num;
    double_step(&val2);

    char* val3 = malloc(7);
    *(int *)(val3 + 5) = 15;
    strange_step(val3);
    free(val3);
    val3 = NULL;

    void* val4 = "123";
    empty_step(val4);

    char* val5 ="uuuu";
    two_step((void*)val5,val5);

    char* val6="hahahahah";
    three_step(val6, val6+2, val6+4);

    char* val71 = "0a";
    char* val72 = "00i";
    char* val73 ="000q";
    step_step_step(val71, val72, val73);

    int b = 65;
    char* a = "A";
    it_may_be_odd(a, b);

    char str[12] = "CS241,CS241";
    tok_step(str);

    void *last = malloc(2);
    *((int *)last) = -85*3;
    the_end(last, last);
    free(last);
    return 0;
}
