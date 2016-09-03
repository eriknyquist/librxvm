#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "rxvm.h"

#include "example_print_err.h"

char *rgx;
char *input;

int main (int argc, char *argv[])
{
    char *gen;
    int ret;
    rxvm_t compiled;

    ret = 0;
    if (argc != 1) {
        printf("Usage: %s <regex> [<num>]\n", argv[0]);
        exit(1);
    }

    srand(time(NULL));

    /* Compile the expression */
    if ((ret = rxvm_compile(&compiled, argv[1])) < 0) {
        example_print_err(ret);
        exit(ret);
    }

    if (compiled.simple) {
        /* If the expression contains no meta-characters, then no
         * instructions will be generated and the "simple" pointer
         * will be populated */
        printf("%s\n", compiled.simple);
    } else {
        gen = rxvm_gen(&compiled, NULL);
        printf("%s\n", gen);
        free(gen);
    }

    rxvm_free(&compiled);
    return ret;
}
