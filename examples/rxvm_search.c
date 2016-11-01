#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"

#include "example_print_err.h"

static void print_substring (char *start, char  *end);

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
        example_print_err(ret);
        exit(ret);
    }

#ifndef NOEXTRAS
    /* print the compiled expression (VM instructions) */
    rxvm_print(&compiled);
#endif /* NOEXTRAS */

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
