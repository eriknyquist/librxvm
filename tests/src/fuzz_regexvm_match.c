#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "regexvm.h"
#include "test_common.h"

char *rgx;
char *input;

#define NUM_ITER     1000

char *testexp[NUM_TESTS_FUZZ_MATCH] = {

    "\\[[0-9]+(\\.[0-9]+)*\\]",
    "aab*(de?(erg)*|qq[B-F]*)*z",
    "\\**\\++(\\??(\\.*([*+.?]*(\\((\\)(\\[)*)*)*)*)*)*",
    "kf?[0Oo3]r*(ty*(d+|ok(.ok)*|yg*yu|uy.+guy|uyg|r(plpl(lpl|p(p|l(lp(u?y|"
    "gu(iu+h(yt*ft(gh|fvuyg(fff)*)*)*j+[a-z])*)*tdrf+y)*)*)*)x.*fxfxfct)*)"
};

int fuzz_regexvm_match (int *count)
{
    regexvm_t compiled;
    const char *msg;
    char *gen;
    int ret;
    int i;
    int j;

    msg = "not ok";
    srand(time(NULL));

    for (i = 0; i < NUM_TESTS_FUZZ_MATCH; ++i) {
        if ((ret = regexvm_compile(&compiled, testexp[i])) < 0) {
            printf("Error (%d) compiling expression %s\n", ret, testexp[i]);
            exit(ret);
        }

        for (j = 0; j < NUM_ITER; ++j) {
            if ((gen = generate_matching_string(&compiled)) == NULL) {
                printf("Error generating string for %s\n", testexp[i]);
                ++ret;
            } else {
                if (regexvm_match(&compiled, gen, 0)) {
                    msg = "ok";
                } else {
                    printf("Matching input %s falsely report non-matching "
                           "against expression %s\n", gen, testexp[i]);
                    ++ret;
                }

                free(gen);
            }
        }

        printf("%s %d %s\n", msg, *count, __func__);
        ++(*count);
        regexvm_free(&compiled);
    }

    return ret;
}
