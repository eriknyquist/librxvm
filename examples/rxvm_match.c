#include <stdio.h>
#include <stdlib.h>
#include "librxvm/rxvm.h"

char *rgx;
char *input;

int main (int argc, char *argv[])
{
    int ret;
    rxvm_t compiled;

    ret = 0;
    if (argc != 3) {
        printf("Usage: %s <regex> <input>\n", argv[0]);
        exit(1);
    }

    /* Compile the expression */
    if ((ret = rxvm_compile(&compiled, argv[1])) < 0) {
#ifndef NOEXTRAS
        rxvm_print_err(ret);
#endif
        exit(ret);
    }

    /* Check if input string matches expression */
    if (rxvm_match(&compiled, argv[2], 0)) {
        printf("Match!\n");
    } else {
        printf("No match.\n");
        ret = 1;
    }

    rxvm_free(&compiled);
    return ret;
}
