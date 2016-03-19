#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"

#define NUMTESTS             9
#define NUMVARIATIONS        5

char *tests[NUMTESTS][(NUMVARIATIONS * 2) + 1] =
{
    {"abc",
        "abc", NULL, NULL, NULL, NULL,
        "ab", "", "abcc", "abcd", "dabc"},

    {"q*",
        "", "q", "qq", "qqq", "qqqqqqqqqqqqqqqqqqqqqqqqqq",
        "qd", "pq", "x", "qqqqqqqqqqqqqqq7qq", NULL},

    {"(c?)+",
        "", "c", "cc", "ccc", "cccccccccccccccccccccccc",
        "ccccccccccccccccd", "cd", "d", NULL, NULL},

    {"[A-Za-z]+",
        "h", "ssefsnfugb", "uHuiBIJHBHgDDb", "AJIJIFHUsxkmskxmskxmsuaaGIC",
        "AlpLPSLPDPLdDUVXVIHEINEBD",
        "", "0", "0490", "$", "}"},

    {"a?b*(cde)+",
        "abcde", "bcde", "cde", "bbcdecde", "bbbbbbbbbbbcdecdecde",
        "aabcde", "abcd", "ab", "a", "cdec"},

    {"a*|b+",
        "a", "aaaaaaaaaa", "", "b", "bbbbbb",
        "ab", "aaaaaaaaaaaaab", "bbbbbbbbba", "ba"},

    {"ab|xyz|q|xx",
        "ab", "xyz", "q", "xx", NULL,
        "abxyzqxx", "abb", "xyza", "xxq", ""},

    {"a(b(c(d(e(f(g)*)*)*)*)*)*",
        "a", "abcdefg", "abcccdddefffggggggggggg", "abccccc", "abbbbbbbbb",
        "abcdefggggggggga", "aa", "accccccccccch", " ", "."},

    {"a*[?*]+|x(yY|Y&(tt*[.+]?|(qq)+)*)",
     "aaa?*???", "xyY", "xY&tqqtt.qqqqtttt.", "xY&tttt+tttqqqqt.qq", "*",
     "a", "xyYY", "xY&?", "xY&tt++", "xY&ttt.qqq"}
};

int test_regexvm(void)
{
    regexvm_t compiled;
    char *msg;
    int ret;
    int total_err;
    int test_err;
    int i;
    int j;

    total_err = 0;
    test_err = 0;
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
                ++test_err;
            }
        }

        /* non-matching input */
        for (j = NUMVARIATIONS + 1; j <= (NUMVARIATIONS * 2); ++j) {
            if (tests[i][j] && regexvm_match(&compiled, tests[i][j])) {
                fprintf(stderr, "Error: non-matching input %s against "
                        "expression %s falsely reports matching input\n",
                        tests[i][j], tests[i][0]);
                ++test_err;
            }
        }

        regexvm_free(&compiled);

        if (test_err) {
            msg = "failed";
            total_err += test_err;
            test_err = 0;
        } else {
            msg = "passed";
        }

        printf("%s: %s %s\n", __func__, msg, tests[i][0]);
    }

    return total_err;
}
