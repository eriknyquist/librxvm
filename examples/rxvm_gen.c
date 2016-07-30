#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "rxvm.h"

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

void rxvm_print_err (int err)
{
    const char *msg;

    switch (err) {
        case RVM_BADOP:
            msg = "Operator used incorrectly";
        break;
        case RVM_BADCLASS:
            msg = "Unexpected character class closing character";
        break;
        case RVM_BADREP:
            msg = "Unexpected closing repetition character";
        break;
        case RVM_BADPAREN:
            msg = "Unexpected parenthesis group closing character";
        break;
        case RVM_EPAREN:
            msg = "Unterminated parenthesis group";
        break;
        case RVM_ECLASS:
            msg = "Unterminated character class";
        break;
        case RVM_EREP:
            msg = "Missing repetition closing character";
        break;
        case RVM_MREP:
            msg = "Empty repetition";
        break;
        case RVM_ETRAIL:
            msg = "Trailing escape character";
        break;
        case RVM_EMEM:
            msg = "Failed to allocate memory";
        break;
        case RVM_EINVAL:
            msg = "Invalid symbol";
        break;
        case RVM_EPARAM:
            msg = "Invalid parameter passed to library function";
        break;
        default:
            msg = "Unrecognised error code";
    }

    printf("Error %d: %s\n", err, msg);
}

int main (int argc, char *argv[])
{
    char *gen;
    int ret;
    rxvm_gencfg_t cfg;
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

    cfg.generosity = 95;
    cfg.whitespace = 0;
    srand(time(NULL));

    /* Compile the expression */
    if ((ret = rxvm_compile(&compiled, argv[1])) < 0) {
        rxvm_print_err(ret);
        exit(ret);
    }

    if (compiled.simple) {
        for (i = 0; i < num; ++i) {
            printf("%s\n", compiled.simple);
        }
    } else {
        for (i = 0; i < num; ++i) {
            gen = rxvm_gen(&compiled, &cfg);
            printf("%s\n", gen);
            free(gen);
        }
    }

    rxvm_free(&compiled);
    return ret;
}
