#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"

typedef struct errtest errtest_t;

struct errtest {
    char *rgx;
    int err;
};

static const errtest_t badop_1 = {
    .rgx = "ab++",
    .err = RXVM_BADOP
};

static const errtest_t badop_2 = {
    .rgx = "ab**",
    .err = RXVM_BADOP
};

static const errtest_t badop_3 = {
    .rgx = "ab??",
    .err = RXVM_BADOP
};

static const errtest_t badop_4 = {
    .rgx = "+",
    .err = RXVM_BADOP
};

static const errtest_t badop_5 = {
    .rgx = "*",
    .err = RXVM_BADOP
};

static const errtest_t badop_6 = {
    .rgx = "?",
    .err = RXVM_BADOP
};

static const errtest_t badop_7 = {
    .rgx = "a{0}",
    .err = RXVM_BADOP
};

static const errtest_t badop_8 = {
    .rgx = "ab(c|d){0}",
    .err = RXVM_BADOP
};

static const errtest_t badop_9 = {
    .rgx = "a{,0}",
    .err = RXVM_BADOP
};

static const errtest_t badop_10 = {
    .rgx = "ab(d*){,0}",
    .err = RXVM_BADOP
};

static const errtest_t badop_11 = {
    .rgx = "{3}",
    .err = RXVM_BADOP
};

static const errtest_t badclass_1 = {
    .rgx = "]",
    .err = RXVM_BADCLASS
};

static const errtest_t badclass_2 = {
    .rgx = "[A-Z]]",
    .err = RXVM_BADCLASS
};

static const errtest_t badclass_3 = {
    .rgx = "q(abc[a-f]])*",
    .err = RXVM_BADCLASS
};

static const errtest_t badclass_4 = {
    .rgx = "(a+(bb*([cde]])*)*)*",
    .err = RXVM_BADCLASS
};

static const errtest_t badclass_5 = {
    .rgx = "dd(r+(fdf(])*)?)+",
    .err = RXVM_BADCLASS
};

static const errtest_t badparen_1 = {
    .rgx = ")",
    .err = RXVM_BADPAREN
};

static const errtest_t badparen_2 = {
    .rgx = "ab)",
    .err = RXVM_BADPAREN
};

static const errtest_t badparen_3 = {
    .rgx = "(a+b?(d)*))",
    .err = RXVM_BADPAREN
};

static const errtest_t badparen_4 = {
    .rgx = "(a?(wde)*dd)+dsdssd)f",
    .err = RXVM_BADPAREN
};

static const errtest_t eparen_1 = {
    .rgx = "(a",
    .err = RXVM_EPAREN
};

static const errtest_t eparen_2 = {
    .rgx = "(a*(bb+(eiof)*dsds)alpalp",
    .err = RXVM_EPAREN
};

static const errtest_t eparen_3 = {
    .rgx = "(a*(bb+(eiof)*dsds)alpalp",
    .err = RXVM_EPAREN
};

static const errtest_t eparen_4 = {
    .rgx = "((aa(bb)*)?",
    .err = RXVM_EPAREN
};

static const errtest_t eclass_1 = {
    .rgx = "[A-Z",
    .err = RXVM_ECLASS
};

static const errtest_t eclass_2 = {
    .rgx = "aa(bb(c[def\\*)*)+ss",
    .err = RXVM_ECLASS
};

static const errtest_t etrail_1 = {
    .rgx = "ss\\",
    .err = RXVM_ETRAIL
};

static const errtest_t badrep_1 = {
    .rgx = "abc}",
    .err = RXVM_BADREP
};

static const errtest_t badrep_2 = {
    .rgx = "abc(def){2,6}}",
    .err = RXVM_BADREP
};

static const errtest_t badrep_3 = {
    .rgx = "}",
    .err = RXVM_BADREP
};

static const errtest_t erep_1 = {
    .rgx = "e+(kn){3",
    .err = RXVM_EREP
};

static const errtest_t erep_2 = {
    .rgx = "a{3,}{",
    .err = RXVM_EREP
};

static const errtest_t erep_3 = {
    .rgx = "a{",
    .err = RXVM_EREP
};

static const errtest_t mrep_1 = {
    .rgx = "frr{}",
    .err = RXVM_MREP
};

static const errtest_t mrep_2 = {
    .rgx = "fw(uu){,}",
    .err = RXVM_MREP
};

static const errtest_t *err_tests[NUM_TESTS_ERR] = {
    &badop_1, &badop_2, &badop_3, &badop_4, &badop_5, &badop_6, &badop_7,
    &badop_8, &badop_9, &badop_10, &badop_11, &badclass_1, &badclass_2,
    &badclass_3, &badclass_4, &badclass_5, &badparen_1, &badparen_2,
    &badparen_3, &badparen_4, &eparen_1, &eparen_2, &eparen_3, &eparen_4,
    &eclass_1, &eclass_2, &etrail_1, &badrep_1, &badrep_2, &badrep_3, &erep_1,
    &erep_2, &erep_3, &mrep_1, &mrep_2
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
        printf("%s: %s #%i\n", msg, __func__, i + 1);
        ++(*count);
    }

    return ret;
}
