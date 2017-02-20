#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rxvm.h"
#include "test_common.h"
#include "randexp.h"

#ifndef NUM_TESTS_FUZZ_FULL_MATCH
#define NUM_TESTS_FUZZ_FULL_MATCH  500
#endif

#define RANDEXP_LIMIT     120
#define RANDEXP_TOKENS    50
#define RANDEXP_LITERALS  25
#define RANDEXP_ESCAPES   25

#define RANDINPUT_LIMIT   500
#define NUM_ITER          1000

static const char *func;

static void log_trs (char *msg, int i)
{
    fprintf(trsfp, ":test-result: %s %s #%d\n", msg, func, i);
    printf("%s: %s #%d\n", msg, func, i);
}

void test_fuzz_full_rxvm_match (void)
{
    char *gen;
    char *exp;
    char sizestr[20];
    rxvm_gencfg_t icfg;
    randexp_cfg_t ecfg;

    rxvm_t compiled;
    uint64_t itersize;
    uint64_t total_size;
    int passed;
    int failed;
    int err;
    int i;
    int j;

    memset(&ecfg, 0, sizeof(randexp_cfg_t));
    init_charmap();
    srand(time(NULL));

    func = __func__;
    total_size = 0;

    ecfg.limit = RANDEXP_LIMIT;
    ecfg.literals = RANDEXP_LITERALS;
    ecfg.tokens = RANDEXP_TOKENS;
    ecfg.escapes = RANDEXP_ESCAPES;

    icfg.limit = RANDINPUT_LIMIT;
    icfg.generosity = 80;
    icfg.whitespace = 10;
    icfg.len = 0;

    for (i = 0; i < NUM_TESTS_FUZZ_FULL_MATCH; ++i) {
        passed = 0;
        failed = 0;
        itersize = 0;

        if ((exp = gen_randexp(&ecfg, NULL)) == NULL) {
            test_err("", "", __func__,
                   "Memory allocation failed during expression generation", 0);
            log_trs("FAIL", i + 1);
            return;
        }

        if ((err = compile_testexp(&compiled, exp)) < 0) {
            test_err(exp, "", __func__, "Compilation failed", err);
            free(exp);
            ++failed;
            goto iter_end;
        }

        for (j = 0; j < NUM_ITER; ++j) {
            if ((gen = rxvm_gen(&compiled, &icfg)) == NULL) {
                test_err(exp, "", __func__,
                        "Memory allocation failed during input generation", 0);
                ++failed;
                free(exp);
                rxvm_free(&compiled);
                goto iter_end;
            } else {
                if (rxvm_match(&compiled, gen, 0)) {
                    ++passed;
                } else {
                    test_err(exp, gen, __func__,
                            "input falsely reported as non-matching", 0);
                    ++failed;
                }

                itersize += icfg.len;
                fflush(stdout);
                free(gen);
            }
        }

        free(exp);
        rxvm_free(&compiled);

iter_end:
        total_size += itersize;
        hrsize(itersize, sizestr, sizeof(sizestr));
        log_trs((failed) ? "FAIL" : "PASS", i + 1);
    }

    hrsize(total_size, sizestr, sizeof(sizestr));
    printf("Total input data used for fuzzing: %s\n", sizestr);
}
