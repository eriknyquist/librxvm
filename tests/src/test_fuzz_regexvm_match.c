#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "regexvm.h"
#include "test_common.h"

char *rgx;
char *input;

#define NUM_ITER     10000

char *testexp[NUM_TESTS_FUZZ_MATCH] = {

    "([\\.]+|\\**)*a?d*;+",

    "abc*(cd?)*",

    "@*(asd+(sd(p|gg(ju)*)+)+)+",

    "a+(r*e+ (y*o+u)* d+r)*u+n*k+\\?*",

    "<.*>.*^string$.*otherstring",

    "<.*>.*^\\(.*\\)$.*^[A-F].+$.*",

    "\\[[0-9]+(\\.[0-9]+)*\\]",

    "aab*(de?(erg)*|qq[B-F]*)*z",

    "\\**\\++(\\??(\\.*([*+.?]*(\\((\\)(\\[)*)*)*)*)*)*",

    "kf?[0Oo3]r*(ty*(d+|ok(.ok)*|yg*yu|uy.+guy|uyg|r(plpl(lpl|p(p|l(lp(u?y|"
    "gu(iu+h(yt*ft(gh|fvuyg(fff)*)*)*j+[a-z])*)*tdrf+y)*)*)*)x.*fxfxfct)*)",

    "(^aa(BB|77|&&|0|f+|(ddx)*)+$.*(cc(dd(EE(FF(gg(hh(II(jj(kk)*)*)*)*)*)*)*"
    ")*)*.*)*",

    "a(b(c(d(e(f(g(h(i(j(k(l(m(n(o(p)*)+)*)+)*)+)*)+)*)+)*)+)*)+)*"
};

static void print_whitespace(char *str)
{
    while (*str) {
        switch (*str) {
            case '\n':
                printf("\\n");
            break;
            case '\r':
                printf("\\r");
            break;
            case '\t':
                printf("\\t");
            break;
            case '\v':
                printf("\\v");
            break;
            default:
                printf("%c", *str);
        }
        ++str;
    }
}

int test_fuzz_regexvm_match (int *count)
{
    regexvm_t compiled;
    const char *msg;
    char *gen;
    char *sizestr;
    uint64_t gensize;
    int ret;
    int i;
    int j;

    msg = "not ok";
    srand(time(NULL));

    for (i = 0; i < NUM_TESTS_FUZZ_MATCH; ++i) {
        gensize = 0;
        if ((ret = compile_testexp(&compiled, testexp[i])) < 0) {
            printf("Error (%d) compiling expression %s\n", ret, testexp[i]);
            exit(ret);
        }

        for (j = 0; j < NUM_ITER; ++j) {
            if ((gen = generate_matching_string(&compiled)) == NULL) {
                printf("Error generating string for %s\n", testexp[i]);
                ++ret;
            } else {
                gensize += sizeof(char) * strlen(gen);
                if (regexvm_match(&compiled, gen, 0)) {
                    msg = "ok";
                } else {
                    printf("Matching input ");
                    print_whitespace(gen);
                    printf(" is falsely reported as non-matching against "
                           "expression %s\n", testexp[i]);
                    ++ret;
                }

                fflush(stdout);
                free(gen);
            }
        }

        sizestr = hrsize(gensize);
        printf("%s %d %s: tested %s of input data\n", msg, *count, __func__,
               sizestr);
        free(sizestr);
        gensize = 0;
        ++(*count);

        regexvm_free(&compiled);
    }

    return ret;
}
