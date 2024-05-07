/**
 * vector
 * CS 241 - Spring 2022
 */
#include "sstring.h"
#include <string.h>
int main(int argc, char *argv[]) {
    // TODO create some tests
    sstring *sstr = cstr_to_sstring("NAME: TOMMY TANG!");
    printf("%s\n\n", sstring_to_cstr(sstr));

    sstring *sstr2 = cstr_to_sstring("AGe :10000 0000? ???");
    sstring *sstr3 = cstr_to_sstring("are you OK!@ #@? ?$* (&^$ ER");

    sstring_append(sstr, sstr);
    sstring_append(sstr, sstr);
    printf("%s\n\n", sstring_to_cstr(sstr));


    vector * v = sstring_split(sstr, ' ');
    printf("{");
    for(size_t i = 0; i<vector_size(v); i++) {
        if(i==vector_size(v)-1) {
            printf("%s}\n", *(char**)vector_at(v,i));
        }
        else {
            printf("%s, ", *(char**)vector_at(v,i));
        }
        
    }

    sstring_substitute(sstr, 0, "TOMMY","");
    printf("%s\n\n", sstring_to_cstr(sstr));

    char* substr = sstring_slice(sstr, 10,23);
    printf("%s\n\n", substr);

    sstring_destroy(sstr);
    sstring_destroy(sstr2);
    sstring_destroy(sstr3);
    vector_destroy(v);
    free(substr);
    return 0;
}
