#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "rxvm.h"

#include "example_print_err.h"

char *rgx;
char *input;

int parse_int (char *input)
{
    int ret;

    ret = 0;
    while (*input) {
        if (*input < '0' || *input > '9')
            return -1;
        ret = (ret * 10) + (*input - '0');
        ++input;
    }

    return ret;
}

int main (int argc, char *argv[])
{
    char *gen;
    int ret;
    rxvm_t compiled;
    int num;
    int i;

    num = 1;
    ret = 0;
    if (argc < 2) {
        printf("Usage: %s <regex> [<num>]\n", argv[0]);
        exit(1);
    }

    if (argc == 3) {
        num = parse_int(argv[2]);
    }

    srand(time(NULL));

    /* Compile the expression */
    if ((ret = rxvm_compile(&compiled, argv[1])) < 0) {
        example_print_err(ret);
        exit(ret);
    }

    if (compiled.simple) {
        for (i = 0; i < num; ++i) {
            printf("%s\n", compiled.simple);
        }
    } else {
        for (i = 0; i < num; ++i) {
            gen = rxvm_gen(&compiled, NULL);
            printf("%s\n", gen);
            free(gen);
        }
    }

    rxvm_free(&compiled);
    return ret;
}
