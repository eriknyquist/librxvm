#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"

#define NUMTESTS             3
#define NUMVARIATIONS        5

const char *tests[NUMTESTS][(NUMVARIATIONS * 2) + 1] =
{
    {"abc",
        "abc", NULL, NULL, NULL, NULL,
        "ab", "", "abcc", "abcd", "dabc"},

    {"q*",
        "", "q", "qq", "qqq", "qqqqqqqqqqqqqqqqqqqqqqqqqq",
        "qd", "pq", "x", "qqqqqqqqqqqqqqq7qq", NULL},

    {"(c?)+",
        "", "c", "cc", "ccc", "cccccccccccccccccccccccc",
        "ccccccccccccccccd", "cd", "d", NULL, NULL}
};

int test_regexvm(void)
{
    regexvm_t compiled;
    int ret;
    int i;
    int j;

    ret = 0;
    for (i = 0; i < NUMTESTS; ++i) {
        if ((ret = regexvm_compile(&compiled, tests[i][0])) < 0) {
            fprintf(stderr, "Error: compilation failed (%d): %s\n",
                    ret, tests[i][0]);
            return ret;
        }

        /* matching input */
        for (j = 1; j <= NUMVARIATIONS; ++j) {
            if (tests[i][j] && !regexvm_match(&compiled, tests[i][j])) {
                fprintf(stderr, "Error: matching input %s against expression "
                        "%s falsely reports non-matching input\n", tests[i][j],
                        tests[i][0]);
                ++ret;
            }
        }

        /* non-matching input */
        for (j = NUMVARIATIONS + 1; j <= (NUMVARIATIONS * 2); ++j) {
            if (tests[i][j] && regexvm_match(&compiled, tests[i][j])) {
                fprintf(stderr, "Error: non-matching input %s against "
                        "expression %s falsely reports matching input\n",
                        tests[i][j], tests[i][0]);
                ++ret;
            }
        }

        regexvm_free(&compiled);
        printf("%s: passed %s\n", __func__, tests[i][0]);
    }

    return ret;
}
