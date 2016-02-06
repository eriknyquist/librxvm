#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"

int main (int argc, char *argv[])
{
    int ret;
    regexvm_t compiled;

    if (argc != 2) {
        printf("Usage: %s <regex>\n", argv[0]);
        exit(1);
    }

    if ((ret = regexvm_compile(&compiled, argv[1])) < 0) {
        regexvm_print_err(ret);
        exit(ret);
    }

    printf("Input expression: %s\n", argv[1]);
    printf("after compilation:\n\n");
    regexvm_print(&compiled);

    regexvm_free(&compiled);
    return 0;
}
