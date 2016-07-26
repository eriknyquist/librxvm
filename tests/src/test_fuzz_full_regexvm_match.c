#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "regexvm.h"
#include "test_common.h"


#define NUM_ITER     100

int test_fuzz_full_regexvm_match (int *count)
{
    const char *msg;
    char *gen;
    char *exp;
    char *sizestr;
    randinput_cfg_t icfg;
    randexp_cfg_t ecfg;

    regexvm_t compiled;
    uint64_t itersize;
    uint64_t total_size;
    int ret;
    int i;
    int j;

    memset(&ecfg, 0, sizeof(randexp_cfg_t));
    init_charmap();
    srand(time(NULL));

    total_size = 0;
    msg = "ok";

    ecfg.tokens = 50;

    icfg.generosity = 80;
    icfg.whitespace = 10;

    for (i = 0; i < NUM_TESTS_FUZZ_FULL_MATCH; ++i) {
        itersize = 0;
        if ((exp = gen_randexp(&ecfg, NULL)) == NULL) {
            test_err("", "", __func__,
                   "Memory allocation failed during expression generation", 0);
            exit(1);
        }

        if ((ret = compile_testexp(&compiled, exp)) < 0) {
            test_err(exp, "", __func__, "Compilation failed", ret);
            free(exp);
            continue;
        }

        for (j = 0; j < NUM_ITER; ++j) {
            if ((gen = gen_randinput(&compiled, &icfg)) == NULL) {
                test_err(exp, "", __func__,
                        "Memory allocation failed during input generation", 0);
                continue;
            } else {
                if (!regexvm_match(&compiled, gen, 0)) {
                    msg = "not ok";
                    test_err(exp, gen, __func__,
                            "input falsely reported as non-matching", 0);
                    ++ret;
                }

                itersize += strlen(gen);
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

        free(exp);
        regexvm_free(&compiled);
    }

    sizestr = hrsize(total_size);
    printf("Total input data used for fuzzing: %s\n", sizestr);
    free(sizestr);
    return ret;
}
