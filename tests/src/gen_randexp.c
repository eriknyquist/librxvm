#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rxvm.h"
#include "string_builder.h"
#include "randexp.h"

#define RANDEXP_LIMIT     120
#define RANDEXP_TOKENS    50
#define RANDEXP_LITERALS  25
#define RANDEXP_ESCAPES   25

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
    randexp_cfg_t cfg;
    char *gen;
    int num;
    int i;

    memset(&cfg, 0, sizeof(randexp_cfg_t));
    cfg.limit = RANDEXP_LIMIT;
    cfg.literals = RANDEXP_LITERALS;
    cfg.tokens = RANDEXP_TOKENS;
    cfg.escapes = RANDEXP_ESCAPES;
    num = 1;

    init_charmap();
    if (argc > 2) {
        printf("Usage: %s <regex> [<num>]\n", argv[0]);
        exit(1);
    }

    if (argc == 2) {
        num = parse_int(argv[1]);
    }

    srand(time(NULL));

    for (i = 0; i < num; ++i) {
        gen = gen_randexp(&cfg, NULL);
        printf("%s\n", gen);
        free(gen);
    }

    return 0;
}
