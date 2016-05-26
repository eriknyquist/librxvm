#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"
#include "test_common.h"

char *rgx;

int main (int argc, char *argv[])
{
    int ret;
    regexvm_t compiled;

    ret = 0;
    if (argc != 2) {
        printf("Usage: %s <regex>\n", argv[0]);
        exit(1);
    }

    /* Compile the expression */
    if ((ret = regexvm_compile(&compiled, argv[1])) < 0) {
        regexvm_print_err(ret);
        exit(ret);
    }

    regexvm_print_oneline(&compiled);

    regexvm_free(&compiled);
    return ret;
}
