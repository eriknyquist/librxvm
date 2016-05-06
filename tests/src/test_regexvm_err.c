#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"
#include "test_common.h"

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

static const errtest_t test_err_badparen_12 = {
    .rgx = ")",
    .err = RVM_BADPAREN
};

static const errtest_t test_err_badparen_13 = {
    .rgx = "ab)",
    .err = RVM_BADPAREN
};

static const errtest_t test_err_badparen_14 = {
    .rgx = "(a+b?(d)*))",
    .err = RVM_BADPAREN
};

static const errtest_t test_err_badparen_15 = {
    .rgx = "(a?(wde)*dd)+dsdssd)f",
    .err = RVM_BADPAREN
};

static const errtest_t test_err_eparen_16 = {
    .rgx = "(a",
    .err = RVM_EPAREN
};

static const errtest_t test_err_eparen_17 = {
    .rgx = "(a*(bb+(eiof)*dsds)alpalp",
    .err = RVM_EPAREN
};

static const errtest_t test_err_eparen_18 = {
    .rgx = "(a*(bb+(eiof)*dsds)alpalp",
    .err = RVM_EPAREN
};

static const errtest_t test_err_eparen_19 = {
    .rgx = "((aa(bb)*)?",
    .err = RVM_EPAREN
};

static const errtest_t test_err_eclass_20 = {
    .rgx = "[A-Z",
    .err = RVM_ECLASS
};

static const errtest_t test_err_eclass_21 = {
    .rgx = "aa(bb(c[def\\*)*)+ss",
    .err = RVM_ECLASS
};

static const errtest_t test_err_etrail_22 = {
    .rgx = "ss\\",
    .err = RVM_ETRAIL
};

static const errtest_t test_err_badrep_23 = {
    .rgx = "abc}",
    .err = RVM_BADREP
};

static const errtest_t test_err_badrep_24 = {
    .rgx = "abc(def){2,6}}",
    .err = RVM_BADREP
};

static const errtest_t test_err_badrep_25 = {
    .rgx = "}",
    .err = RVM_BADREP
};

static const errtest_t test_err_erep_26 = {
    .rgx = "e+(kn){3",
    .err = RVM_EREP
};

static const errtest_t test_err_erep_27 = {
    .rgx = "a{3,}{",
    .err = RVM_EREP
};

static const errtest_t test_err_erep_28 = {
    .rgx = "a{",
    .err = RVM_EREP
};

static const errtest_t test_err_mrep_29 = {
    .rgx = "frr{}",
    .err = RVM_MREP
};

static const errtest_t test_err_mrep_30 = {
    .rgx = "fw(uu){,}",
    .err = RVM_MREP
};

static const errtest_t test_err_badop_31 = {
    .rgx = "{3}",
    .err = RVM_BADOP
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

int test_regexvm_err (int *count)
{
    regexvm_t compiled;
    const errtest_t *test;
    const char *msg;
    int ret;
    int err;
    int i;

    ret = 0;
    for (i = 0; i < NUM_TESTS_ERR; ++i) {
        test = err_tests[i];
        if ((err = compile_testexp(&compiled, test->rgx)) == test->err) {
            msg = "ok";
        } else {
            fprintf(stderr, "On compilation of regex %s:\ngot return code %d,"
                            " expecting %d\n", test->rgx, err, test->err);
            msg = "not ok";
            ++ret;
        }

        if (err == 0)
                regexvm_free(&compiled);

        printf("%s %d %s\n", msg, *count, __func__);
        ++(*count);
    }

    return ret;
}
