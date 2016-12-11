#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rxvm.h"
#include "test_common.h"

char *rgx;
char *input;

#define RANDINPUT_LIMIT   500
#define NUM_ITER          1000

char *testexp[NUM_TESTS_FUZZ_MATCH] = {

    "([\\.]+|\\**)*a?d*;+",

    "abc*(cd?)*",

    "abc*(cd[0-9]+)*",

    ".[^^v0-9]+)*",

    "abc*(cd[0-9]+|[^A-F])*",

    "@*(asd+(sd(p|gg(ju)*)+)+)+",

    "a+(r*e+ (y*o+u)* d+r)*u+n*k+\\?*",

    "<.*>.*^string$.*otherstring",

    "<.*>.*^\\(.*\\)$.*^[A-F].+$.*",

    "(.*\\)$.*[^A-F].+[0-9a-f]+",

    "\\[[0-9]+(\\.[0-9]{17,18})*\\]",

    "aab*(de?(erg)*|qq[B-F]*)*z",

    "aab*(de?(erg)*|qq[^B-F]*)*z",

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

int test_fuzz_rxvm_match (int *count)
{
    const char *msg;
    char *gen;
    char *sizestr;
    rxvm_gencfg_t cfg;

    rxvm_t compiled;
    uint64_t itersize;
    uint64_t total_size;
    int ret;
    int passed;
    int failed;
    int i;
    int j;

    ret = 0;
    total_size = 0;
    srand(time(NULL));

    cfg.limit = RANDINPUT_LIMIT;
    cfg.generosity = 50;
    cfg.whitespace = 10;

    for (i = 0; i < NUM_TESTS_FUZZ_MATCH; ++i) {
        itersize = 0;
        passed = 0;
        failed = 0;

        if ((ret = compile_testexp(&compiled, testexp[i])) < 0) {
            test_err(testexp[i], "", __func__, "Compilation failed", ret);
            ++failed;
            goto end_iter;
        }

        for (j = 0; j < NUM_ITER; ++j) {
            if ((gen = rxvm_gen(&compiled, &cfg)) == NULL) {
                test_err(testexp[i], "", __func__,
                        "Memory allocation failed during input generation", 0);
                ++failed;
                rxvm_free(&compiled);
                goto end_iter;
            } else {
                if (rxvm_match(&compiled, gen, 0)) {
                    msg = "PASS";
                    ++passed;
                } else {
                    msg = "FAIL";
                    test_err(testexp[i], gen, __func__,
                            "input falsely reported as non-matching", 0);
                    ++failed;
                }

                fprintf(trsfp, ":test-result: %s %s #%d.%d\n", msg, __func__, i, j);
                itersize += strlen(gen);
                fflush(stdout);
                free(gen);
            }
        }
        rxvm_free(&compiled);

end_iter:
        total_size += itersize;
        ret += failed;
        sizestr = hrsize(itersize);
        fprintf(logfp, "%s #%d, %s of test data generated\n", __func__, i, sizestr);
        free(sizestr);
        ++(*count);
    }

    sizestr = hrsize(total_size);
    fprintf(logfp, "Total input data used for fuzzing: %s\n", sizestr);
    free(sizestr);
    return ret;
}
