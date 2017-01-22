#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "librxvm/rxvm.h"

char *rgx;

static int get_containing_line (FILE *fp, char *linebuf, size_t bufsize,
                                uint64_t match_size)
{
    char c;
    long start;

    start = ftell(fp);
    fseek(fp, -1, SEEK_CUR);

    /* rxvm_fsearch() sets the file pointer at the first character of
     * the matching text; in order to print the whole line, we must
     * backup to last newline character */
    while ((c = fgetc(fp)) != '\n' && ftell(fp) > 1) {
        fseek(fp, -2, SEEK_CUR);
    }

    /* Print the whole line */
    if (fgets(linebuf, bufsize, fp) != linebuf) {
        return -1;
    }

    /* Reset file pointer to the first character after the matching
     * text, so we can continue to search the file with further
     * calls to rxvm_fsearch */
    fseek(fp, start + match_size, SEEK_SET);
    return 0;
}

int main (int argc, char *argv[])
{
    char linebuf[120];
    FILE *fp;
    uint64_t size;
    int i;
    int ret;
    int flags;
    rxvm_t compiled;

    ret = 0;
    if (argc < 3) {
        printf("Usage: %s <regex> <filename>\n", argv[0]);
        exit(1);
    }

    /* Compile the expression */
    if ((ret = rxvm_compile(&compiled, argv[1])) < 0) {
        rxvm_print_err(ret);
        exit(ret);
    }

    flags = RXVM_MULTILINE;

    for (i = 2; i < argc; ++i) {
        /* Open the file */
        if ((fp = fopen(argv[i], "r")) == NULL) {
            printf("Error opening file %s. Exiting.\n", argv[2]);
            exit(1);
        }

        /* Find all occurrences of compiled expression in file */
        while (rxvm_fsearch(&compiled, fp, &size, flags)) {
            if ((get_containing_line(fp, linebuf, sizeof(linebuf), size)) < 0) {
                printf("Error reading matching text from file '%s'\n", argv[2]);
                exit(1);
            }

            if (argc > 3) {
                printf("%s: %s", argv[i], linebuf);
            } else {
                printf("%s", linebuf);
            }
        }

        fclose(fp);
    }

    rxvm_free(&compiled);
    return ret;
}
