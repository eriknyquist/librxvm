#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "regexvm.h"

char *rgx;

static void regexvm_print_err (int err);

static char *get_matching_text (FILE *fp, uint64_t size)
{
    char *ret;

    if ((ret = malloc(size + 1)) == NULL) {
        return NULL;
    }

    if (fread(ret, sizeof(char), size, fp) != (sizeof(char) * size)) {
        return NULL;
    }

    ret[size] = '\0';
    return ret;
}

int main (int argc, char *argv[])
{
    FILE *fp;
    uint64_t size;
    int i;
    int ret;
    char *match;
    regexvm_t compiled;

    ret = 0;
    if (argc != 3) {
        printf("Usage: %s <regex> <filename>\n", argv[0]);
        exit(1);
    }

    /* Compile the expression */
    if ((ret = regexvm_compile(&compiled, argv[1])) < 0) {
        regexvm_print_err(ret);
        exit(ret);
    }

    /* Open the file */
    if ((fp = fopen(argv[2], "r")) == NULL) {
        printf("Error opening file %s. Exiting.\n", argv[2]);
        exit(1);
    }

    /* Find all occurrences of compiled expression in file */
    while (regexvm_fsearch(&compiled, fp, &size, 0)) {
        if ((match = get_matching_text(fp, size)) == NULL) {
            printf("Error reading matching text from file '%s'\n", argv[2]);
            exit(1);
        }

        printf("Found match: %s\n", match);
        free(match);
    }

    fclose(fp);
    regexvm_free(&compiled);
    return ret;
}

void regexvm_print_err (int err)
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
        default:
            msg = "Unrecognised error code";
    }

    printf("Error %d: %s\n", err, msg);
}
