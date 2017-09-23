#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxvm.h"
#include "test_common.h"

#define RANDINPUT_LIMIT   200
#define NUM_ITER          100

char *rgx;
char *input;

static uint64_t total_size;
static rxvm_gencfg_t cfg;
static int tests;
static const char *func;

static void log_trs (char *msg, int i)
{
    fprintf(trsfp, ":test-result: %s %s #%d\n", msg, func, i);
    printf("%s: %s #%d\n", msg, func, i);
}

static void report_iter_size (int i, char *size)
{
    fprintf(logfp, "%s #%d, %s of test data generated\n", func, i, size);
}

static void fuzz_rxvm_match (char *regex)
{
    rxvm_t compiled;
    char sizestr[20];
    uint64_t itersize;
    char *msg, *gen;
    int ret, j;
    int percent;

    itersize = 0;
    percent = 0;

    msg = "PASS";
    j = 0;
    ++tests;

    if ((ret = compile_testexp(&compiled, regex)) < 0) {
        printf("compile failed\n");
        test_err(regex, "", func, "Compilation failed", ret);
        log_trs("FAIL", tests);
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

            percent = j / (NUM_ITER / 100);
            printf("  %d%%\r", percent);
        }
    }

    log_trs(msg, tests);
    rxvm_free(&compiled);
    total_size += itersize;
    hrsize(itersize, sizestr, sizeof(sizestr));
    report_iter_size(tests, sizestr);
}

void test_fuzz_rxvm_match (void)
{
    tests = 0;
    total_size = 0;
    func = __func__;

    cfg.limit = RANDINPUT_LIMIT;
    cfg.generosity = 90;
    cfg.whitespace = 10;

    fuzz_rxvm_match("a*b*c*d");
    fuzz_rxvm_match("a+|b?|c*|d{2,}|e+|f?|g*");
    fuzz_rxvm_match("(a+|b?|c*|d{2,}|e+|f?|g*)+");
    fuzz_rxvm_match("(a+|b?|c*|d{2,}|e+|f?|g*){56,97}");
    fuzz_rxvm_match("([\\.]+|\\**)*a?d*;+");
    fuzz_rxvm_match("abc*(cd?)*");
    fuzz_rxvm_match("abc*(cd[0-9]+)*");
    fuzz_rxvm_match(".([^^v0-9]+)*");
    fuzz_rxvm_match("abc*(cd[0-9]+|[^A-F])*");
    fuzz_rxvm_match("@*(asd+(sd(p|gg(ju)*)+)+)+");
    fuzz_rxvm_match("a+(r*e+ (y*o+u)* d+r)*u+n*k+\\?*");
    fuzz_rxvm_match("<.*>.*string.*otherstring");
    fuzz_rxvm_match("<.*>.*\\(.*\\).*[A-F].+.*");
    fuzz_rxvm_match("(.*\\\\).*[^A-F].+[0-9a-f]+");
    fuzz_rxvm_match("\\[[0-9]+(\\.[0-9]{17,18})*\\]");
    fuzz_rxvm_match("aab*(de?(erg)*|qq[B-F]*)*z");
    fuzz_rxvm_match("aab*(de?(erg)*|qq[^B-F]*)*z");

    fuzz_rxvm_match("\\**\\++(\\??(\\.*([*+.?]{,5}(\\((\\)(\\[)*)*)*)*)*)*");

    fuzz_rxvm_match(
        "kf?[0Oo3]r*(ty*(d+|ok(.ok)|yg*yu|uy.+guy|uyg|r(plpl(lpl|p(p|l(lp(u?y|"
        "gu(iu+h(yt*ft(gh|fvuyg(fff)*)*)*j+[a-z])*)tdrf+y)*)*)*)x.*fxfxfct)*)");

    fuzz_rxvm_match(
        "(aa(BB|77|&&|0|f+|(ddx)*)+.*(cc(dd(EE(FF(gg(hh(II(jj(kk)*)*)*)*)*)*)*"
        ")*)*.*)*");

    fuzz_rxvm_match(
        "a(b(c(d(e(f(g(h(i(j(k(l(m(n(o(p)*)+)*)+)*)+)*)+)*)+)*)+)*)+)*");

    fuzz_rxvm_match("abc{4,5}");
    fuzz_rxvm_match("xyz{,56}");
    fuzz_rxvm_match("c(d(e){,4}){2,6}");
    fuzz_rxvm_match("abc*(de+f{4,67}|xxyy){,14}");
    fuzz_rxvm_match("dd(eA{3,4}){6,8}");
    fuzz_rxvm_match("b{2,6}(f?p){4}(h(e(y){5,6}){3,8}){5}");
    fuzz_rxvm_match("d(E{2,3}){2,3}");
    fuzz_rxvm_match("bb(cc(dd(EE.{3,4}){6,}){2,}){8,9}");
}
