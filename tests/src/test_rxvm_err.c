#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"

typedef struct errtest errtest_t;

struct errtest {
    char *rgx;
    int err;
};

static const errtest_t test_err_badop_1 = {
    .rgx = "ab++",
    .err = RXVM_BADOP
};

static const errtest_t test_err_badop_2 = {
    .rgx = "ab**",
    .err = RXVM_BADOP
};

static const errtest_t test_err_badop_3 = {
    .rgx = "ab??",
    .err = RXVM_BADOP
};

static const errtest_t test_err_badop_4 = {
    .rgx = "+",
    .err = RXVM_BADOP
};

static const errtest_t test_err_badop_5 = {
    .rgx = "*",
    .err = RXVM_BADOP
};

static const errtest_t test_err_badop_6 = {
    .rgx = "?",
    .err = RXVM_BADOP
};

static const errtest_t test_err_badclass_7 = {
    .rgx = "]",
    .err = RXVM_BADCLASS
};

static const errtest_t test_err_badclass_8 = {
    .rgx = "[A-Z]]",
    .err = RXVM_BADCLASS
};

static const errtest_t test_err_badclass_9 = {
    .rgx = "q(abc[a-f]])*",
    .err = RXVM_BADCLASS
};

static const errtest_t test_err_badclass_10 = {
    .rgx = "(a+(bb*([cde]])*)*)*",
    .err = RXVM_BADCLASS
};

static const errtest_t test_err_badclass_11 = {
    .rgx = "dd(r+(fdf(])*)?)+",
    .err = RXVM_BADCLASS
};

static const errtest_t test_err_badparen_12 = {
    .rgx = ")",
    .err = RXVM_BADPAREN
};

static const errtest_t test_err_badparen_13 = {
    .rgx = "ab)",
    .err = RXVM_BADPAREN
};

static const errtest_t test_err_badparen_14 = {
    .rgx = "(a+b?(d)*))",
    .err = RXVM_BADPAREN
};

static const errtest_t test_err_badparen_15 = {
    .rgx = "(a?(wde)*dd)+dsdssd)f",
    .err = RXVM_BADPAREN
};

static const errtest_t test_err_eparen_16 = {
    .rgx = "(a",
    .err = RXVM_EPAREN
};

static const errtest_t test_err_eparen_17 = {
    .rgx = "(a*(bb+(eiof)*dsds)alpalp",
    .err = RXVM_EPAREN
};

static const errtest_t test_err_eparen_18 = {
    .rgx = "(a*(bb+(eiof)*dsds)alpalp",
    .err = RXVM_EPAREN
};

static const errtest_t test_err_eparen_19 = {
    .rgx = "((aa(bb)*)?",
    .err = RXVM_EPAREN
};

static const errtest_t test_err_eclass_20 = {
    .rgx = "[A-Z",
    .err = RXVM_ECLASS
};

static const errtest_t test_err_eclass_21 = {
    .rgx = "aa(bb(c[def\\*)*)+ss",
    .err = RXVM_ECLASS
};

static const errtest_t test_err_etrail_22 = {
    .rgx = "ss\\",
    .err = RXVM_ETRAIL
};

static const errtest_t test_err_badrep_23 = {
    .rgx = "abc}",
    .err = RXVM_BADREP
};

static const errtest_t test_err_badrep_24 = {
    .rgx = "abc(def){2,6}}",
    .err = RXVM_BADREP
};

static const errtest_t test_err_badrep_25 = {
    .rgx = "}",
    .err = RXVM_BADREP
};

static const errtest_t test_err_erep_26 = {
    .rgx = "e+(kn){3",
    .err = RXVM_EREP
};

static const errtest_t test_err_erep_27 = {
    .rgx = "a{3,}{",
    .err = RXVM_EREP
};

static const errtest_t test_err_erep_28 = {
    .rgx = "a{",
    .err = RXVM_EREP
};

static const errtest_t test_err_mrep_29 = {
    .rgx = "frr{}",
    .err = RXVM_MREP
};

static const errtest_t test_err_mrep_30 = {
    .rgx = "fw(uu){,}",
    .err = RXVM_MREP
};

static const errtest_t test_err_badop_31 = {
    .rgx = "{3}",
    .err = RXVM_BADOP
};

static const errtest_t *err_tests[NUM_TESTS_ERR] = {
    &test_err_badop_1, &test_err_badop_2, &test_err_badop_3, &test_err_badop_4,
    &test_err_badop_5, &test_err_badop_6, &test_err_badclass_7,
    &test_err_badclass_8, &test_err_badclass_9, &test_err_badclass_10,
    &test_err_badclass_11, &test_err_badparen_12, &test_err_badparen_13,
    &test_err_badparen_14, &test_err_badparen_15, &test_err_eparen_16,
    &test_err_eparen_17, &test_err_eparen_18, &test_err_eparen_19,
    &test_err_eclass_20, &test_err_eclass_21, &test_err_etrail_22,
    &test_err_badrep_23, &test_err_badrep_24, &test_err_badrep_25,
    &test_err_erep_26, &test_err_erep_27, &test_err_erep_28, &test_err_mrep_29,
    &test_err_mrep_30, &test_err_badop_31
};

int test_rxvm_err (int *count)
{
    rxvm_t compiled;
    const errtest_t *test;
    const char *msg;
    int ret;
    int err;
    int i;

    ret = 0;
    for (i = 0; i < NUM_TESTS_ERR; ++i) {
        test = err_tests[i];
        if ((err = compile_testexp(&compiled, test->rgx)) == test->err) {
            msg = "PASS";
        } else {
            fprintf(logfp, "On compilation of regex %s:\ngot return code %d,"
                            " expecting %d\n", test->rgx, err, test->err);
            msg = "FAIL";
            ++ret;
        }

        if (err == 0)
                rxvm_free(&compiled);

        fprintf(trsfp, ":test-result: %s %s #%d\n", msg, __func__, *count);
        ++(*count);
    }

    return ret;
}
