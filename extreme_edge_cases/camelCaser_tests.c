/**
 * extreme_edge_cases
 * CS 241 - Spring 2022
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"
#include <string.h>

int compareRes(char** expected, char** toTest) {
    if(expected ==NULL && toTest == NULL) {
        return 1;
    } else if (expected == NULL && toTest != NULL) {
        return 0;
    } else if (expected != NULL && toTest == NULL) {
        return 0;
    }

    while(*expected!=NULL && *toTest!=NULL) {
        if(strcmp(*expected,*toTest)) {
            return 0;
        }
        expected++;
        toTest++;
    }
    if(*expected!=NULL || *toTest!=NULL) {
        return 0;
    }
    return 1;
}


int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Implement me!
    
    
    // CASE 1
    const char* test_input1 = "";
    char** res1 = camelCaser(test_input1);
    char* expected1[] = {NULL};
    if(!compareRes(expected1, res1)) {
        destroy(res1);
        res1 = NULL;
        return 0;
    }
    destroy(res1);
    res1 = NULL;
    
    // CASE 2
    const char* test_input2 = "   ";
    char** res2 = camelCaser(test_input2);
    char* expected2[] = {NULL};
    if(!compareRes(expected2, res2)) {
        destroy(res2);
        res2 = NULL;
        return 0;
    }
    destroy(res2);
    res2 = NULL;

    // CASE 3
    const char* test_input3 = ".. .....   ";
    char** res3 = camelCaser(test_input3);
    char* expected3[] = {"", "", "", "", "", "", "", NULL};
    if(!compareRes(expected3, res3)) {
        destroy(res3);
        res3 = NULL;
        return 0;
    }
    destroy(res3);
    res3 = NULL;

    // CASE 4
    char *test_input4 = "  .d. .   asd ds..... 你好，啊asdf 啊 aas d啊ha ha h. Hello world!!! ashgjk,{hjf(90-10 0=-as fds)";
    char** res4 = camelCaser(test_input4);
    char* expected4[] = {"", "d", "", "asdDs", "", "", "", "", "你好，啊asdf啊AasD啊haHaH", "helloWorld", "", "", "ashgjk", "", "hjf", "90", "100", "", "asFds", NULL};
    if(!compareRes(expected4, res4)) {
        destroy(res4);
        res4 = NULL;
        return 0;
    }
    destroy(res4);
    res4 = NULL;

    // CASE 5
    char *test_input5 = "asd as shd dfa *dgf ,ds aa v32 f312.  // .  sd f?4sd1 /...     3 452 fdfh l sd gdf   dr o. gds,gs::'d  fgh [34fd  f857 823we2df d";
    char** res5 = camelCaser(test_input5);
    char* expected5[] = {"asdAsShdDfa","dgf","dsAaV32F312","","","","sdF","4sd1","","","","3452FdfhLSdGdfDrO","gds","gs","","","dFgh",NULL};
    if(!compareRes(expected5, res5)) {
        destroy(res5);
        res5 = NULL;
        return 0;
    }
    destroy(res5);
    res5 = NULL;

    // CASE 6
    char *test_input6 = "asgfhsdg9843t5u4esjfnsldkugq384";
    char** res6 = camelCaser(test_input6);
    char* expected6[] = {NULL};
    if(!compareRes(expected6, res6)) {
        destroy(res6);
        res6 = NULL;
        return 0;
    }
    destroy(res6);
    res6 = NULL;

    // CASE 7
    char *test_input7 = NULL;
    char** res7 = camelCaser(test_input7);
    if(!compareRes(NULL, res7)) {
        return 0;
    }


    // CASE 8
    char *test_input8 = "asas.<we11 12. ewfw sd 2>df gf .#ads $1 h sf: d13 |sdg 98f d4524. 3t5f u4e sjf  ,; na sd a& 1*1# 1#$^2 ^ dsldku gq384";
    char** res8 = camelCaser(test_input8);
    char* expected8[] = {"asas",
        "",
        "we1112",
        "ewfwSd2",
        "dfGf",
        "",
        "ads",
        "1HSf",
        "d13",
        "sdg98FD4524",
        "3t5fU4eSjf",
        "",
        "naSdA",
        "1",
        "1",
        "1",
        "",
        "",
        "2",
        NULL};
    if(!compareRes(expected8, res8)) {
        destroy(res8);
        res8 = NULL;
        return 0;
    }
    destroy(res8);
    res8 = NULL;

    // CASE 9
    char *test_input9 = "12,>,13 4..5**4 3^5=891 3:{}|24 21 ..a 4.141 59- 10 = 20 -23 ../472 89~ !";
    char** res9 = camelCaser(test_input9);
    char* expected9[] = {        "12",
        "",
        "",
        "134",
        "",
        "5",
        "",
        "43",
        "5",
        "8913",
        "",
        "",
        "",
        "2421",
        "",
        "a4",
        "14159",
        "10",
        "20",
        "23",
        "",
        "",
        "47289",
        "",
        NULL};
    if(!compareRes(expected9, res9)) {
        destroy(res9);
        res9 = NULL;
        return 0;
    }
    destroy(res9);
    res9 = NULL;
    return 1;
}
