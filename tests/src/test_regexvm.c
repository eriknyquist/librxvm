#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"

#define NUMTESTS             13
#define NUMVARIATIONS        5

char *tests[NUMTESTS][(NUMVARIATIONS * 2) + 1] =
{
    {"abc",
        "abc", NULL, NULL, NULL, NULL,
        "ab", "", "abcc", "abcd", "dabc"},

    {"ab|",
        "ab", "", NULL, NULL, NULL,
        "abb", "b", "a", "x", NULL},

    {"q*",
        "", "q", "qq", "qqqqqq", "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq",
        "qd", "pq", "x", "qqqqqqqqqqqqqqq7qq", NULL},

    {"(c?)+",
        "", "c", "cc", "ccc", "cccccccccccccccccccccccccccccccccccccccccccccc",
        "ccccccccccccccccd", "cd", "d", NULL, NULL},

    {"[A-Za-z]+",
        "h", "ssefsnfugb", "uHuiBIJHBHgDDb", "AJIJIFHUsxskmskxmsqkoxmqsuaaGIC",
        "dyRHdilhjHRJiFIfniFNJgzrdgggrhyjfjfrrrrrhhhthhnzmrhdlIUKJFghfKMUFNIN",
        "", "0", "0490", "$", "}"},

    {"a?b*(cde)+",
        "abcde", "bcde", "cde", "bbcdecde", "abbbbbbbbbbbbbbbbbbbbbbcdecdecde",
        "aabcde", "abcd", "ab", "a", "cdec"},

    {"a*|b+",
        "a", "aaaaaaaaaa", "", "b", "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
        "ab", "aaaaaaaaaaaaab", "bbbbbbbbba", "ba"},

    {"ab|xyz|q|xx",
        "ab", "xyz", "q", "xx", NULL,
        "abxyzqxx", "abb", "xyza", "xxq", ""},

    {"a(b(c(d(e(f(g)*)*)*)*)*)*",
        "a", "abcdefg", "abcccdddefffggggggggggg", "abccccccccc", "abbbbbbbbb",
        "abcdefggggggggga", "aa", "accccccccccch", " ", "."},

    {"a*[?*]+|x(yY|Y&(tt*[.+]?|(qq)+)*)",
     "aaa?*???", "xyY", "xY&tqqtt.qqqqtttt.", "xY&tttttttt+tttqqqqqqt.qq", "*",
     "a", "xyYY", "xY&?", "xY&tt++", "xY&ttt.qqq"},

    {"qwerty(\\*+\\+*[*?.+])*",
        "qwerty*.*?", "qwerty******+.*?*?*?*?*?", "qwerty", "qwerty****++++++",
        "qwerty***************************************",
        "qwert", "qwerty+", "qwerty*+..", "qwerty*?+", "qwertyy"},

    {"[A-Fa-f@0-9]+",
        "A", "AbCdEfFfF", "aB0F75EEeeEe12@3456ffff", "@", "9",
        "G", "", "abcdefg", "AbCdEFz", "+"},

    {"[\t\r\v\f\b]\n\a\n\\?\\*\\+\\?\\\\[+*?.]",
        "\t\n\a\n?*+?\\+", "\r\n\a\n?*+?\\*", "\v\n\a\n?*+?\\?",
        "\f\n\a\n?*+?\\.", "\b\n\a\n?*+?\\.",
        "b\n\a\n?*+?\\?", "\t\n\an?*+?\\+", "\t\n\a\n?*+?\\++"}
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

        printf("%s: test %d %s\n", __func__, i + 1, msg);
    }

    return total_err;
}
