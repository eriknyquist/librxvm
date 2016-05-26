#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "regexvm.h"
#include "test_common.h"

char *rgx;
char *input;

int main (int argc, char *argv[])
{
    char *gen;
    int ret;
    regexvm_t compiled;

    ret = 0;
    if (argc != 2) {
        printf("Usage: %s <regex>\n", argv[0]);
        exit(1);
    }

    srand(time(NULL));
    /* Compile the expression */
    if ((ret = regexvm_compile(&compiled, argv[1])) < 0) {
        regexvm_print_err(ret);
        exit(ret);
    }

    gen = generate_matching_string(&compiled);
    printf("%s\n", gen);

    free(gen);
    regexvm_free(&compiled);
    return ret;
}
