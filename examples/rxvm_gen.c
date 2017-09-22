#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "librxvm/rxvm.h"

char *rgx;
char *input;

int main (int argc, char *argv[])
{
    char *gen;
    int ret, num, i;
    rxvm_t compiled;

    ret = 0;
    num = 1;

    if (argc < 2 || argc > 3) {
        printf("Usage: %s <regex> [<number>]\n", argv[0]);
        exit(1);
    }

    if (argc == 3) {
        num = atoi(argv[2]);
    }

    srand(time(NULL));

    /* Compile the expression */
    if ((ret = rxvm_compile(&compiled, argv[1])) < 0) {
        rxvm_print_err(ret);
        exit(ret);
    }

    for (i = 0; i < num; ++i) {
        gen = rxvm_gen(&compiled, NULL);
        printf("%s\n", gen);
        free(gen);
    }

    rxvm_free(&compiled);
    return ret;
}
