#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"

#define NUMTESTS             11

typedef struct errtest errtest_t;

struct errtest {
    char *rgx;
    int err;
};

static const errtest_t test_err_badop_1 = {
    .rgx = "ab++",
    .err = RVM_BADOP
};

static const errtest_t test_err_badop_2 = {
    .rgx = "ab**",
    .err = RVM_BADOP
};

static const errtest_t test_err_badop_3 = {
    .rgx = "ab??",
    .err = RVM_BADOP
};

static const errtest_t test_err_badop_4 = {
    .rgx = "+",
    .err = RVM_BADOP
};

static const errtest_t test_err_badop_5 = {
    .rgx = "*",
    .err = RVM_BADOP
};

static const errtest_t test_err_badop_6 = {
    .rgx = "?",
    .err = RVM_BADOP
};

static const errtest_t test_err_badclass_7 = {
    .rgx = "]",
    .err = RVM_BADCLASS
};

static const errtest_t test_err_badclass_8 = {
    .rgx = "[A-Z]]",
    .err = RVM_BADCLASS
};

static const errtest_t test_err_badclass_9 = {
    .rgx = "q(abc[a-f]])*",
    .err = RVM_BADCLASS
};

static const errtest_t test_err_badclass_10 = {
    .rgx = "(a+(bb*([cde]])*)*)*",
    .err = RVM_BADCLASS
};

static const errtest_t test_err_badclass_11 = {
    .rgx = "dd(r+(fdf(])*)?)+",
    .err = RVM_BADCLASS
};

static const errtest_t *tests[NUMTESTS] = {
    &test_err_badop_1, &test_err_badop_2, &test_err_badop_3, &test_err_badop_4,
    &test_err_badop_5, &test_err_badop_6, &test_err_badclass_7,
    &test_err_badclass_8, &test_err_badclass_9, &test_err_badclass_10,
    &test_err_badclass_11
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

        printf("%s: test %d %s\n", __func__, i + 1,
            (ret) ? "failed" : "passed");
    }

    return ret;
}
