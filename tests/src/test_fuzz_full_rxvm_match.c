#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rxvm.h"
#include "test_common.h"

#define RANDEXP_LIMIT     80
#define RANDINPUT_LIMIT   500
#define NUM_ITER          1000

static int tests;

void test_fuzz_full_rxvm_match (void)
{
    const char *msg;
    char *gen;
    char *exp;
    char sizestr[20];
    rxvm_gencfg_t icfg;
    randexp_cfg_t ecfg;

    rxvm_t compiled;
    uint64_t itersize;
    uint64_t total_size;
    int ret;
    int passed;
    int failed;
    int err;
    int i;
    int j;

    memset(&ecfg, 0, sizeof(randexp_cfg_t));
    init_charmap();
    srand(time(NULL));

    ret = 0;
    total_size = 0;
    msg = "ok";

    ecfg.limit = RANDEXP_LIMIT;
    ecfg.tokens = 80;

    icfg.limit = RANDINPUT_LIMIT;
    icfg.generosity = 80;
    icfg.whitespace = 10;
    icfg.len = 0;

    tests = 0;

    for (i = 0; i < NUM_TESTS_FUZZ_FULL_MATCH; ++i) {
        passed = 0;
        failed = 0;
        itersize = 0;
        ++tests;

        if ((exp = gen_randexp(&ecfg, NULL)) == NULL) {
            test_err("", "", __func__,
                   "Memory allocation failed during expression generation", 0);
            exit(1);
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
                    msg = "not ok";
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
        ret += failed;
        hrsize(itersize, sizestr, sizeof(sizestr));
        printf("%s %d %s: %d failed, %d passed (out of %d) %s\n", msg, tests,
                __func__, failed, passed, NUM_ITER, sizestr);
    }

    hrsize(total_size, sizestr, sizeof(sizestr));
    printf("Total input data used for fuzzing: %s\n", sizestr);
}
