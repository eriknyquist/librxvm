#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rxvm.h"
#include "test_common.h"

#define RANDINPUT_LIMIT   100
#define NUM_ITER          1000

char *rgx;
char *input;

static uint64_t total_size;
static rxvm_gencfg_t cfg;
static int tests;

static void log_trs (char *msg, const char *func, int i)
{
    fprintf(trsfp, ":test-result: %s %s #%d\n", msg, func, i);
    printf("%s: %s #%d\n", msg, func, i);
}

static void report_iter_size (const char *func, int i, char *size)
{
    fprintf(logfp, "%s #%d, %s of test data generated\n", func, i, size);
}

static void fuzz_rxvm_match (char *regex, const char *func)
{
    rxvm_t compiled;
    char sizestr[20];
    uint64_t itersize;
    char *msg, *gen;
    int ret, j;

    itersize = 0;

    msg = "PASS";
    j = 0;
    ++tests;

    if ((ret = compile_testexp(&compiled, regex)) < 0) {
        printf("compile failed\n");
        test_err(regex, "", func, "Compilation failed", ret);
        log_trs("FAIL", func, tests);
        return;
    }

    for (j = 0; j < NUM_ITER; ++j) {
        if ((gen = rxvm_gen(&compiled, &cfg)) == NULL) {
            test_err(regex, "", func,
                    "Memory allocation failed during input generation", 0);
            msg = "FAIL";
            rxvm_free(&compiled);
            break;
        } else {
            if (!rxvm_match(&compiled, gen, 0)) {
                test_err(regex, gen, func,
                        "input falsely reported as non-matching", 0);
                msg = "FAIL";
                break;
            } else {
                itersize += strlen(gen);
                fflush(stdout);
                free(gen);
            }
        }
    }

    log_trs(msg, func, tests);
    rxvm_free(&compiled);
    total_size += itersize;
    hrsize(itersize, sizestr, sizeof(sizestr));
    report_iter_size(func, tests, sizestr);
}

void test_fuzz_rxvm_match (void)
{
    tests = 0;
    total_size = 0;
    srand(time(NULL));

    cfg.limit = RANDINPUT_LIMIT;
    cfg.generosity = 90;
    cfg.whitespace = 10;

    fuzz_rxvm_match("a*b*c*d", __func__);
    fuzz_rxvm_match("a+|b?|c*|d{2,}|e+|f?|g*", __func__);
    fuzz_rxvm_match("(a+|b?|c*|d{2,}|e+|f?|g*)+", __func__);
    fuzz_rxvm_match("(a+|b?|c*|d{2,}|e+|f?|g*){56,97}", __func__);
    fuzz_rxvm_match("([\\.]+|\\**)*a?d*;+", __func__);
    fuzz_rxvm_match("abc*(cd?)*", __func__);
    fuzz_rxvm_match("abc*(cd[0-9]+)*", __func__);
    fuzz_rxvm_match(".([^^v0-9]+)*", __func__);
    fuzz_rxvm_match("abc*(cd[0-9]+|[^A-F])*", __func__);
    fuzz_rxvm_match("@*(asd+(sd(p|gg(ju)*)+)+)+", __func__);
    fuzz_rxvm_match("a+(r*e+ (y*o+u)* d+r)*u+n*k+\\?*", __func__);
    fuzz_rxvm_match("<.*>.*string.*otherstring", __func__);
    fuzz_rxvm_match("<.*>.*\\(.*\\).*[A-F].+.*", __func__);
    fuzz_rxvm_match("(.*\\\\).*[^A-F].+[0-9a-f]+", __func__);
    fuzz_rxvm_match("\\[[0-9]+(\\.[0-9]{17,18})*\\]", __func__);
    fuzz_rxvm_match("aab*(de?(erg)*|qq[B-F]*)*z", __func__);
    fuzz_rxvm_match("aab*(de?(erg)*|qq[^B-F]*)*z", __func__);

    fuzz_rxvm_match("\\**\\++(\\??(\\.*([*+.?]{,5}(\\((\\)(\\[)*)*)*)*)*)*",
        __func__);

    fuzz_rxvm_match(
        "kf?[0Oo3]r*(ty*(d+|ok(.ok)|yg*yu|uy.+guy|uyg|r(plpl(lpl|p(p|l(lp(u?y|"
        "gu(iu+h(yt*ft(gh|fvuyg(fff)*)*)*j+[a-z])*)tdrf+y)*)*)*)x.*fxfxfct)*)",
        __func__);

    fuzz_rxvm_match(
        "(aa(BB|77|&&|0|f+|(ddx)*)+.*(cc(dd(EE(FF(gg(hh(II(jj(kk)*)*)*)*)*)*)*"
        ")*)*.*)*", __func__);

    fuzz_rxvm_match(
        "a(b(c(d(e(f(g(h(i(j(k(l(m(n(o(p)*)+)*)+)*)+)*)+)*)+)*)+)*)+)*",
        __func__);

    fuzz_rxvm_match("abc{4,5}", __func__);
    fuzz_rxvm_match("xyz{,56}", __func__);
    fuzz_rxvm_match("c(d(e){,4}){2,6}", __func__);
    fuzz_rxvm_match("abc*(de+f{4,67}|xxyy){,14}", __func__);
    fuzz_rxvm_match("dd(eA{3,4}){6,8}", __func__);
    fuzz_rxvm_match("b{2,6}(f?p){4}(h(e(y){5,6}){3,8}){5}", __func__);
    fuzz_rxvm_match("d(E{2,3}){2,3}", __func__);
    fuzz_rxvm_match("bb(cc(dd(EE.{3,4}){6,}){2,}){8,9}", __func__);
}
