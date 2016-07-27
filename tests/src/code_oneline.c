#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"

char *rgx;

int main (int argc, char *argv[])
{
    int ret;
    rxvm_t compiled;

    ret = 0;
    if (argc != 2) {
        printf("Usage: %s <regex>\n", argv[0]);
        exit(1);
    }

    /* Compile the expression */
    if ((ret = rxvm_compile(&compiled, argv[1])) < 0) {
        rxvm_print_err(ret);
        exit(ret);
    }

    rxvm_print_oneline(&compiled);

    rxvm_free(&compiled);
    return ret;
}
