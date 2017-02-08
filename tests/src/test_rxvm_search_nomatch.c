#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"

static int tests;
static const char *func;

void verify_search_nomatch (char *regex, char *input)
{
    rxvm_t compiled;
    const char *msg;
    char *start, *end;
    int ret;
    int err;
    int i;

    ret = 0;
    ++tests;

    if ((err = compile_testexp(&compiled, regex)) < 0) {
        fprintf(logfp, "Error compiling regex %s\n", regex);
        ++ret;
    } else {
        if (rxvm_search(&compiled, input, &start, &end, 0)) {
            fprintf(logfp, "Error- input %s wrongly reported as containing match "
                   "to expression %s\n", input, regex);
            ++ret;
        } else {
            if (start != NULL || end != NULL) {
                fprintf(logfp, "Error- start and end pointers are not NULL after "
                       "no match for expression %s was found in input "
                       "%s.\n", regex, input);
                ++ret;
            }
        }

        rxvm_free(&compiled);
    }

    msg = (ret) ? "FAIL" : "PASS";
    fprintf(trsfp, ":test-result: %s %s #%d\n", msg, func, tests);
    printf("%s: %s #%i\n", msg, func, i + 1);
}

void test_rxvm_search_nomatch (void)
{

    tests = 0;
    func = __func__;

    verify_search_nomatch("xy", ",czy7*%^&0-(spo()6(%^&nowirgbi");
    verify_search_nomatch("xy", ",czy7*%^&0-(xpo(x6(%^&xowirxYbi");
    verify_search_nomatch("abc*", "897654788888898888888888886");
    verify_search_nomatch("abcc*", "897ab654788ab88988888888888ab");
    verify_search_nomatch("abc+", "897ab654788ab88988888888888ab");
    verify_search_nomatch("joiu{2,6}", "@@@#@##@IUOYTRYEUYTRUIOP{UY^++TR%$%");
    verify_search_nomatch("joiu{2,6}", "@joiuIUOYTRYEUYTRUIOP{UY^++TR%$%");
    verify_search_nomatch("joiu{5,6}", "@joiuuuuIUOYTRYEUYTRUIOP{UY^++TR%$%");
    verify_search_nomatch("a(bb|kl{6,76}(88)*){99}", "aaaabbbbccccddddeeeeffff");
    verify_search_nomatch("aa|bb|cc|dd", "ababacadacabdcadcbdcdabcdbdca");
}
