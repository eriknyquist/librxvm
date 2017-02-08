#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"

static int tests;
static const char *func;

static void verify_errcode (char *rgx, int errcode)
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
    func = __func__;
    tests = 0;

    verify_errcode("ab++", RXVM_BADOP);
    verify_errcode("ab**", RXVM_BADOP);
    verify_errcode("ab??", RXVM_BADOP);
    verify_errcode("+", RXVM_BADOP);
    verify_errcode("*", RXVM_BADOP);
    verify_errcode("?", RXVM_BADOP);
    verify_errcode("a{0}", RXVM_BADOP);
    verify_errcode("ab(c|d){0}", RXVM_BADOP);
    verify_errcode("a{,0}", RXVM_BADOP);
    verify_errcode("ab(d*){,0}", RXVM_BADOP);
    verify_errcode("{3}", RXVM_BADOP);

    verify_errcode("]", RXVM_BADCLASS);
    verify_errcode("[A-Z]]", RXVM_BADCLASS);
    verify_errcode("q(abc[a-f]])*", RXVM_BADCLASS);
    verify_errcode("(a+(bb*([cde]])*)*)*", RXVM_BADCLASS);
    verify_errcode("dd(r+(fdf(])*)?)+", RXVM_BADCLASS);

    verify_errcode("abc}", RXVM_BADREP);
    verify_errcode("abc(def){2,6}}", RXVM_BADREP);
    verify_errcode("}", RXVM_BADREP);

    verify_errcode(")", RXVM_BADPAREN);
    verify_errcode("ab)", RXVM_BADPAREN);
    verify_errcode("(a+b?(d)*))", RXVM_BADPAREN);
    verify_errcode("(a?(wde)*dd)+dsdssd)f", RXVM_BADPAREN);

    verify_errcode("(a", RXVM_EPAREN);
    verify_errcode("(a*(bb+(eiof)*dsds)alpalp", RXVM_EPAREN);
    verify_errcode("((aa(bb)*)?", RXVM_EPAREN);

    verify_errcode("[A-Z", RXVM_ECLASS);
    verify_errcode("aa(bb(c[def\\*)*)+ss", RXVM_ECLASS);

    verify_errcode("e+(kn){3", RXVM_EREP);
    verify_errcode("a{3,}{", RXVM_EREP);
    verify_errcode("a{", RXVM_EREP);

    verify_errcode("[-]", RXVM_ERANGE);
    verify_errcode("[a-]", RXVM_ERANGE);
    verify_errcode("[-Z]", RXVM_ERANGE);
    verify_errcode("a+|bb?|[-]", RXVM_ERANGE);
    verify_errcode("abcd[a-]", RXVM_ERANGE);
    verify_errcode("rr*[-Z]", RXVM_ERANGE);

    verify_errcode("ab{5,,}", RXVM_ECOMMA);
    verify_errcode("ab{,,7}", RXVM_ECOMMA);
    verify_errcode("ab{5,6,}", RXVM_ECOMMA);
    verify_errcode("ab{5,6,7}", RXVM_ECOMMA);

    verify_errcode("ab{a}", RXVM_EDIGIT);
    verify_errcode("ab{5,y}", RXVM_EDIGIT);
    verify_errcode("ab{,y}", RXVM_EDIGIT);
    verify_errcode("ab{5.2}", RXVM_EDIGIT);
    verify_errcode("ab{4,5.6}", RXVM_EDIGIT);
    verify_errcode("ab{{5}", RXVM_EDIGIT);
    verify_errcode("ab{{", RXVM_EDIGIT);

    verify_errcode("\\", RXVM_ETRAIL);
    verify_errcode("ss\\", RXVM_ETRAIL);
    verify_errcode("(a\\", RXVM_ETRAIL);
    verify_errcode("(a*(bb+(eiof)*dsds)a\\", RXVM_ETRAIL);
    verify_errcode("((aa(bb)*)?\\", RXVM_ETRAIL);

    verify_errcode("frr{}", RXVM_MREP);
    verify_errcode("fw(uu){,}", RXVM_MREP);

    verify_errcode("☆", RXVM_EINVAL);
    verify_errcode("ਈ", RXVM_EINVAL);
    verify_errcode("ߞ", RXVM_EINVAL);
    verify_errcode("ߞ☆☂Ȫਈ", RXVM_EINVAL);
    verify_errcode("abc☆", RXVM_EINVAL);
    verify_errcode("abc(d+|qq?)fg☆", RXVM_EINVAL);
    verify_errcode("c+ਈ", RXVM_EINVAL);
    verify_errcode("dd|ff|rr|ߞ", RXVM_EINVAL);
    verify_errcode("abcfߞ☆☂Ȫਈ", RXVM_EINVAL);
}
