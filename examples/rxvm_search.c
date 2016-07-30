#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"

static void print_substring (char *start, char  *end);
static void rxvm_print_err (int err);

int main (int argc, char *argv[])
{
    char *start;
    char *end;
    int ret;
    rxvm_t compiled;

    ret = 0;
    if (argc != 3) {
        printf("Usage: %s <regex> <input>\n", argv[0]);
        exit(1);
    }

    /* Compile the expression */
    if ((ret = rxvm_compile(&compiled, argv[1])) < 0) {
        rxvm_print_err(ret);
        exit(ret);
    }

    /* print the compiled expression (VM instructions) */
    rxvm_print(&compiled);

    /* Check if input string matches expression */
    if (rxvm_search(&compiled, argv[2], &start, &end, 0)) {
        printf("Match!\n");
        printf("Found matching substring:\n");
        print_substring(start, end);
    } else {
        printf("No match.\n");
        ret = 1;
    }

    rxvm_free(&compiled);
    return ret;
}

void print_substring (char *start, char *end)
{
    while (start != end) {
        printf("%c", *start);
        ++start;
    }
    printf("\n");
}

void rxvm_print_err (int err)
{
    const char *msg;

    switch (err) {
        case RXVM_BADOP:
            msg = "Operator used incorrectly";
        break;
        case RXVM_BADCLASS:
            msg = "Unexpected character class closing character";
        break;
        case RXVM_BADREP:
            msg = "Unexpected closing repetition character";
        break;
        case RXVM_BADPAREN:
            msg = "Unexpected parenthesis group closing character";
        break;
        case RXVM_EPAREN:
            msg = "Unterminated parenthesis group";
        break;
        case RXVM_ECLASS:
            msg = "Unterminated character class";
        break;
        case RXVM_EREP:
            msg = "Missing repetition closing character";
        break;
        case RXVM_MREP:
            msg = "Empty repetition";
        break;
        case RXVM_ETRAIL:
            msg = "Trailing escape character";
        break;
        case RXVM_EMEM:
            msg = "Failed to allocate memory";
        break;
        case RXVM_EINVAL:
            msg = "Invalid symbol";
        break;
        case RXVM_EPARAM:
            msg = "Invalid parameter passed to library function";
        break;
        default:
            msg = "Unrecognised error code";
    }

    printf("Error %d: %s\n", err, msg);
}
