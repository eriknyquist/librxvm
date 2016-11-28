#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "rxvm.h"

#include "example_print_err.h"

char *rgx;

static char *get_matching_text (FILE *fp, uint64_t size)
{
    char *ret;

    if ((ret = malloc(size + 1)) == NULL) {
        return NULL;
    }

    if (fread(ret, sizeof(char), size, fp) != size) {
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
    int flags;
    char *match;
    rxvm_t compiled;

    ret = 0;
    if (argc != 3) {
        printf("Usage: %s <regex> <filename>\n", argv[0]);
        exit(1);
    }

    /* Compile the expression */
    if ((ret = rxvm_compile(&compiled, argv[1])) < 0) {
        example_print_err(ret);
        exit(ret);
    }

    /* Open the file */
    if ((fp = fopen(argv[2], "r")) == NULL) {
        printf("Error opening file %s. Exiting.\n", argv[2]);
        exit(1);
    }

    flags = RXVM_MULTILINE;
    /* Find all occurrences of compiled expression in file */
    while (rxvm_fsearch(&compiled, fp, &size, flags)) {
        if ((match = get_matching_text(fp, size)) == NULL) {
            printf("Error reading matching text from file '%s'\n", argv[2]);
            exit(1);
        }

        printf("Found match: %s\n", match);
        free(match);
    }

    fclose(fp);
    rxvm_free(&compiled);
    return ret;
}
