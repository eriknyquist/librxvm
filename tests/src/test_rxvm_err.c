#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"

static int tests;

static void verify_errcode (char *rgx, int errcode, const char *func)
{
    rxvm_t compiled;
    const char *msg;
    int err;

    msg = "PASS";
    ++tests;

    if ((err = compile_testexp(&compiled, rgx)) != errcode) {
        fprintf(logfp, "On compilation of regex %s:\ngot return code %d,"
                            " expecting %d\n", rgx, err, errcode);
        msg = "FAIL";
    }

    if (err == 0)
        rxvm_free(&compiled);

    fprintf(trsfp, ":test-result: %s %s #%d\n", msg, func, tests);
    printf("%s: %s #%i\n", msg, func, tests);
}

void test_rxvm_err (void)
{
    tests = 0;
    verify_errcode("ab++", RXVM_BADOP, __func__);
    verify_errcode("ab**", RXVM_BADOP, __func__);
    verify_errcode("ab??", RXVM_BADOP, __func__);
    verify_errcode("+", RXVM_BADOP, __func__);
    verify_errcode("*", RXVM_BADOP, __func__);
    verify_errcode("?", RXVM_BADOP, __func__);
    verify_errcode("a{0}", RXVM_BADOP, __func__);
    verify_errcode("ab(c|d){0}", RXVM_BADOP, __func__);
    verify_errcode("a{,0}", RXVM_BADOP, __func__);
    verify_errcode("ab(d*){,0}", RXVM_BADOP, __func__);
    verify_errcode("{3}", RXVM_BADOP, __func__);

    verify_errcode("]", RXVM_BADCLASS, __func__);
    verify_errcode("[A-Z]]", RXVM_BADCLASS, __func__);
    verify_errcode("q(abc[a-f]])*", RXVM_BADCLASS, __func__);
    verify_errcode("(a+(bb*([cde]])*)*)*", RXVM_BADCLASS, __func__);
    verify_errcode("dd(r+(fdf(])*)?)+", RXVM_BADCLASS, __func__);

    verify_errcode(")", RXVM_BADPAREN, __func__);
    verify_errcode("ab)", RXVM_BADPAREN, __func__);
    verify_errcode("(a+b?(d)*))", RXVM_BADPAREN, __func__);
    verify_errcode("(a?(wde)*dd)+dsdssd)f", RXVM_BADPAREN, __func__);

    verify_errcode("(a", RXVM_EPAREN, __func__);
    verify_errcode("(a*(bb+(eiof)*dsds)alpalp", RXVM_EPAREN, __func__);
    verify_errcode("((aa(bb)*)?", RXVM_EPAREN, __func__);

    verify_errcode("[A-Z", RXVM_ECLASS, __func__);
    verify_errcode("aa(bb(c[def\\*)*)+ss", RXVM_ECLASS, __func__);

    verify_errcode("\\", RXVM_ETRAIL, __func__);
    verify_errcode("ss\\", RXVM_ETRAIL, __func__);

    verify_errcode("abc}", RXVM_BADREP, __func__);
    verify_errcode("abc(def){2,6}}", RXVM_BADREP, __func__);
    verify_errcode("}", RXVM_BADREP, __func__);

    verify_errcode("e+(kn){3", RXVM_EREP, __func__);
    verify_errcode("a{3,}{", RXVM_EREP, __func__);
    verify_errcode("a{", RXVM_EREP, __func__);

    verify_errcode("frr{}", RXVM_MREP, __func__);
    verify_errcode("fw(uu){,}", RXVM_MREP, __func__);
}
