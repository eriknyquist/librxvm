#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "regexvm.h"
#include "test_common.h"

char *rgx;
char *input;

#define NUM_ITER     1000

char *testexp[NUM_TESTS_FUZZ_MATCH] = {

    "([\\.]+|\\**)*a?d*;+",

    "abc*(cd?)*",

    "@*(asd+(sd(p|gg(ju)*)+)+)+",

    "a+(r*e+ (y*o+u)* d+r)*u+n*k+\\?*",

    "<.*>.*^string$.*otherstring",

    "<.*>.*^\\(.*\\)$.*^[A-F].+$.*",

    "\\[[0-9]+(\\.[0-9]{17,18})*\\]",

    "aab*(de?(erg)*|qq[B-F]*)*z",

    "\\**\\++(\\??(\\.*([*+.?]{,5}(\\((\\)(\\[)*)*)*)*)*)*",

    "kf?[0Oo3]r*(ty*(d+|ok(.ok)|yg*yu|uy.+guy|uyg|r(plpl(lpl|p(p|l(lp(u?y|"
    "gu(iu+h(yt*ft(gh|fvuyg(fff)*)*)*j+[a-z])*)tdrf+y)*)*)*)x.*fxfxfct)*)",

    "(^aa(BB|77|&&|0|f+|(ddx)*)+$.*(cc(dd(EE(FF(gg(hh(II(jj(kk)*)*)*)*)*)*)*"
    ")*)*.*)*",

    "a(b(c(d(e(f(g(h(i(j(k(l(m(n(o(p)*)+)*)+)*)+)*)+)*)+)*)+)*)+)*",

    "abc{4,5}",

    "xyz{,56}",

    "c(d(e){,4}){2,6}",

    "abc*(de+f{4,67}|xxyy){,14}",

    "dd(eA{3,4}){6,8}",

    "b{2,6}(f?p){4}(h(e(y){5,6}){3,8}){5}",

    "d(E{2,3}){2,3}",

    "bb(cc(dd(EE.{3,4}){6,}){2,}){8,9}"
};

int test_fuzz_regexvm_match (int *count)
{
    const char *msg;
    char *gen;
    char *sizestr;
    randinput_cfg_t cfg;

    regexvm_t compiled;
    uint64_t gensize;
    uint64_t itersize;
    uint64_t total_size;
    int ret;
    int i;
    int j;

    total_size = 0;
    msg = "ok";
    srand(time(NULL));

    for (i = 0; i < NUM_TESTS_FUZZ_MATCH; ++i) {
        itersize = 0;
        if ((ret = compile_testexp(&compiled, testexp[i])) < 0) {
            test_err(testexp[i], "", __func__, "Compilation failed", ret);
            exit(ret);
        }
        cfg.compiled = &compiled;

        for (j = 0; j < NUM_ITER; ++j) {
            if ((gen = gen_randinput(&cfg, &gensize)) == NULL) {
                test_err(testexp[i], "", __func__,
                        "Memory allocation failed during input generation", 0);
                ++ret;
            } else {
                if (!regexvm_match(&compiled, gen, 0)) {
                    msg = "not ok";
                    test_err(testexp[i], gen, __func__,
                            "input falsely reported as non-matching", 0);
                    ++ret;
                }

                itersize += gensize;
                fflush(stdout);
                free(gen);
            }
        }

        total_size += itersize;
        sizestr = hrsize(itersize);
        printf("%s %d %s: %d failed, %d passed, %s\n", msg, *count, __func__,
               ret, NUM_ITER - ret, sizestr);
        free(sizestr);
        ++(*count);

        regexvm_free(&compiled);
    }

    sizestr = hrsize(total_size);
    printf("Total input data used for fuzzing: %s\n", sizestr);
    free(sizestr);
    return ret;
}
