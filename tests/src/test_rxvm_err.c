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
    verify_errcode("ab?{3}", RXVM_BADOP);
    verify_errcode("+ab", RXVM_BADOP);
    verify_errcode("*ab", RXVM_BADOP);
    verify_errcode("?ab", RXVM_BADOP);
    verify_errcode("{3}ab", RXVM_BADOP);
    verify_errcode("+", RXVM_BADOP);
    verify_errcode("*", RXVM_BADOP);
    verify_errcode("?", RXVM_BADOP);
    verify_errcode("a{0}", RXVM_BADOP);
    verify_errcode("ab(c|d){0}", RXVM_BADOP);
    verify_errcode("a{,0}", RXVM_BADOP);
    verify_errcode("ab(d*){,0}", RXVM_BADOP);
    verify_errcode("{3}", RXVM_BADOP);
    verify_errcode("asregsgri+kfeog+keor+f+g+fef+b++", RXVM_BADOP);
    verify_errcode("abbbbbbb*b*bbbbbbb*Bbbbbbb*Bbb**", RXVM_BADOP);
    verify_errcode("aiiiiii?i?i?i*i++i*i*i+b??", RXVM_BADOP);
    verify_errcode("abcdbc{5}G{7}HGHGfhgOP{6}b{3}{3}", RXVM_BADOP);
    verify_errcode("+atttttttttttttttttttttttttttttb", RXVM_BADOP);
    verify_errcode("*ab590876{{{{[[[[((((97857857856", RXVM_BADOP);
    verify_errcode("?avvvvvvvpvpvpvpvvpvvvvvvvvvvvvb", RXVM_BADOP);
    verify_errcode("{3}a8766*&&%^*^**(&*&^*(*^&**&^b", RXVM_BADOP);

    verify_errcode("]", RXVM_BADCLASS);
    verify_errcode("][A-Z]", RXVM_BADCLASS);
    verify_errcode("[A-Z]]", RXVM_BADCLASS);
    verify_errcode("abc[a-f]]", RXVM_BADCLASS);
    verify_errcode("q(abc[a-f]])*", RXVM_BADCLASS);
    verify_errcode("(a+(bb*([cde]])*)*)*", RXVM_BADCLASS);
    verify_errcode("dd(r+(fdf(])*)?)+", RXVM_BADCLASS);
    verify_errcode("f+g*[r-z][a-F][f-*]*]", RXVM_BADCLASS);
    verify_errcode("aaaaaa[a-f][4-%]][A-Z]", RXVM_BADCLASS);
    verify_errcode("[3-#][5-)][A-Z]]", RXVM_BADCLASS);

    verify_errcode("abc}", RXVM_BADREP);
    verify_errcode("def{2,6}}", RXVM_BADREP);
    verify_errcode("def}{2,6}", RXVM_BADREP);
    verify_errcode("abc(def){2,6}}", RXVM_BADREP);
    verify_errcode("d{4}(gy){4,5}fdfdfdfd{4,}abc}", RXVM_BADREP);
    verify_errcode("de{,6}hjiogjhghjg99htf{2,6}}", RXVM_BADREP);
    verify_errcode("d&*&^ef}{{{{{2,6}", RXVM_BADREP);
    verify_errcode("}", RXVM_BADREP);

    verify_errcode(")", RXVM_BADPAREN);
    verify_errcode("ab)", RXVM_BADPAREN);
    verify_errcode("(a+b?(d)*))", RXVM_BADPAREN);
    verify_errcode("(a+b?)(d)*)", RXVM_BADPAREN);
    verify_errcode(")(a+b?)(d)*", RXVM_BADPAREN);
    verify_errcode("(a?(wde)*dd)+dsdssd)f", RXVM_BADPAREN);

    verify_errcode("(a", RXVM_EPAREN);
    verify_errcode("x(a", RXVM_EPAREN);
    verify_errcode("x(a+(f|g", RXVM_EPAREN);
    verify_errcode("(a*(bb+(eiof)*dsds)alpalp", RXVM_EPAREN);
    verify_errcode("((aa(bb)*)?", RXVM_EPAREN);

    verify_errcode("[A-Z", RXVM_ECLASS);
    verify_errcode("[a-f][A-Z", RXVM_ECLASS);
    verify_errcode("[a-f][6-^][A-Z", RXVM_ECLASS);
    verify_errcode("fighoisdrgoig[A-Z", RXVM_ECLASS);
    verify_errcode("dgdrggdrg[a-f][A-Z", RXVM_ECLASS);
    verify_errcode("drgdrgdrd[a-f][6-^][A-Z", RXVM_ECLASS);
    verify_errcode("aa(bb(c[def\\*)*)+ss", RXVM_ECLASS);

    verify_errcode("e+(kn){3", RXVM_EREP);
    verify_errcode("a{3,}{", RXVM_EREP);
    verify_errcode("a{", RXVM_EREP);
    verify_errcode("f{4}ff{6,}l{,6}e+(kn){3", RXVM_EREP);
    verify_errcode("f{4}ff{6,}l{,6}{", RXVM_EREP);

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
    verify_errcode("ab{5,,})){{", RXVM_ECOMMA);
    verify_errcode("ab{,,7}****", RXVM_ECOMMA);
    verify_errcode("ab{5,6,}???", RXVM_ECOMMA);
    verify_errcode("ab{5,6,7}+++", RXVM_ECOMMA);
    verify_errcode("q{,3}uu{4}a{3,4}b{5,,}", RXVM_ECOMMA);
    verify_errcode("ia{4}g{5,}oo{,7}ab{,,7}", RXVM_ECOMMA);
    verify_errcode("abi{4}d{,6};;{0,}'{5,6,}", RXVM_ECOMMA);
    verify_errcode("a{4}6{,3}##{5,}b{5,6,7}", RXVM_ECOMMA);

    verify_errcode("ab{a}", RXVM_EDIGIT);
    verify_errcode("ab{5\\}", RXVM_EDIGIT);
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
    verify_errcode("frr{}}}}", RXVM_MREP);
    verify_errcode("fw(uu){,}***", RXVM_MREP);
    verify_errcode("f{3}r{3,5}r{}{{{}}", RXVM_MREP);
    verify_errcode("fw{3}(u{4,5}u){,})))))))))", RXVM_MREP);

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
