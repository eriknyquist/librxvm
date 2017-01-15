#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"

char *nomatch_tests[NUM_TESTS_NOMATCH][2] =
{
    {"xy", ",czy7*%^&0-(spo()6(%^&nowirgbi"},
    {"abc*", "897654788888898888888888886"},
    {"joiu{2,6}", "@@@#@##@IUOYTRYEYUIOPUYTRUIOP{UY^++TR%$%"},
    {"a(bb|kl{6,76}(88)*){99}", "aaaabbbbccccddddeeeeffff"},
    {"aa|bb|cc|dd", "ababacadacabdcadcbdcdabcdbdca"}
};

int test_rxvm_search_nomatch (int *count)
{
    rxvm_t compiled;
    const char *msg;
    char *start, *end, *regex, *input;
    int ret;
    int err;
    int i;

    for (i = 0; i < NUM_TESTS_NOMATCH; ++i) {
        ret = 0;
        regex = nomatch_tests[i][0];
        input = nomatch_tests[i][1];

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
        fprintf(trsfp, ":test-result: %s %s #%d\n", msg, __func__, *count);
        printf("%s: %s #%i\n", msg, __func__, i + 1);
        ++(*count);
    }

    return ret;
}
