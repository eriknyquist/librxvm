#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"

#define NUMTESTS             2

typedef struct errtest errtest_t;

struct errtest {
    char *rgx;
    int err;
};

static const errtest_t test_err_1 = {
    .rgx = "+a",
    .err = RVM_BADOP
};

static const errtest_t test_err_2 = {
    .rgx = "ab**",
    .err = RVM_BADOP
};

static const errtest_t *tests[NUMTESTS] = {
    &test_err_1, &test_err_2
};

int test_regexvm_err (void)
{
    regexvm_t compiled;
    const errtest_t *test;
    int ret;
    int err;
    int i;

    ret = 0;
    for (i = 0; i < NUMTESTS; ++i) {
        test = tests[i];
        if ((err = regexvm_compile(&compiled, test->rgx)) != test->err) {
            fprintf(stderr, "On compilation of regex %s:\ngot return code %d,"
                            " expecting %d\n", test->rgx, err, test->err);
            ret++;
        }

        if (err == 0)
                regexvm_free(&compiled);

        printf("%s: %s %s\n", __func__, (ret) ? "failed" : "passed", test->rgx);
    }

    return ret;
}
