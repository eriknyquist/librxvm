#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"

static int tests;

static char *multi_tests[NUM_TESTS_SEARCH_MULTI][4] =
{
    {"x+", "oki9u08y72389)**IOxxxxokKIIjJoixxImIOKmOIOuyyRKj", "xxxx", "xx"},

    {"erik+", "erierierierierierierierierikkkkerikeri", "erikkkk", "erik"},

    {"x+", "oki9u08y72389)*Y9*YO(U*IOxokKIIjJojxxxOKmbbggOIOuRKj", "x", "xxx"},

    {"aa*b+c?", "09t5ol55llll5taaaaabbb0pppo0yiy5ptiabcim", "aaaaabbb", "abc"},

    {"aa*b+c?", "09taaa5aoaaal55laaal5tabc0pppo0yiy5abyi0im5om", "abc", "ab"},

    {"ab(cd(ef)?)*|qtqt|ux*", "OPIPkIOIkpOI(*Ijoiab99877uxxi90op", "ab", "uxx"},

    {"ab(cd(ef)?)*|qtqt|ux*", "OPIPkIOuIkpOI(*Ijoiab998779op9op90", "u", "ab"},

    {"ab(cd(ef)?)*|qtqt|ux*", "OPIPkIOuIkp7uxxx", "u", "uxxx"},

    {"ab(cd(ef)?)*|qtqt|ux*", "uxxxxjw098hqtqtpoefhisr", "uxxxx", "qtqt"},

    {"ab(cd(ef)?)*|qtqt|ux*", "OPIPkIa99abcdefcdcd877u90", "abcdefcdcd", "u"},

    {"xy(ab(cd(ef(gh)*)*)*)*", "_+)(*%&%$xyabcdefgh0xymk", "xyabcdefgh", "xy"},

    {"xy(ab(cd(ef(gh)*)*)*)*", "xyaqtxyabababab_=__)+&*6", "xy", "xyabababab"},

    {"xy(ab(cd(ef(gh)*)*)*)*", "@|@||@{xyabcdefef_=_xy6", "xyabcdefef", "xy"},

    {"([0-9]\\.?)+", "kopM<L.,loIMOi192.168gggg12.33JUO", "192.168", "12.33"},

    {"([0-9]\\.?)+", "kopiNmKM<L.,loIMOiiiiii44iiiiA1JUIOIO", "44", "1"},

    {"([0-9]\\.?)+", "KDJIWDoieOFIWOIF66.78EJofej9AN", "66.78", "9"},

    {"word|xy(regex([Vv][Mm]|stuff)?)*", "869xyioKioword*&guijiIOJiou", "xy",
        "word"},

    {"word|xy(regex([Vv][Mm]|stuff)?)*",
        "869i83498l3ko.word........KioxyregexVMjiIOJiou", "word", "xyregexVM"},

    {"word|xy(regex([Vv][Mm]|stuff)?)*", "869ioKioxy*ixyregexVm&guijiIOJiou",
        "xy", "xyregexVm"},

    {"word|xy(regex([Vv][Mm]|stuff)?)*",
        "ixy869ioKiopskoJh9q89j98djxyregexVMregexregexstuff*&guijiIOJiou",
        "xy", "xyregexVMregexregexstuff"}
};

void test_rxvm_search_multi (void)
{
    rxvm_t compiled;
    const char *msg;
    char *start;
    char *end;
    int ret;
    int err;
    int i;

    tests = 0;

    for (i = 0; i < NUM_TESTS_SEARCH_MULTI; ++i) {
        ret = 0;
        ++tests;

        if ((err = compile_testexp(&compiled, multi_tests[i][0])) < 0) {
            fprintf(logfp, "Error compiling regex %s\n", multi_tests[i][0]);
            ++ret;
        } else {
            if (rxvm_search(&compiled, multi_tests[i][1], &start, &end, 0)) {
                if (substring_match(multi_tests[i][2], start, end)) {
                } else {
                    fprintf(logfp, "Error matching regex %s\n", multi_tests[i][0]);
                    fprintf(logfp, "Expecting \"%s\", but substring pointers show \"",
                           multi_tests[i][2]);
                    do {
                        fprintf(logfp, "%c", *start);
                    } while (start++ != end);

                    fprintf(logfp, "\"\n");
                    ++ret;
                }
            } else {
                fprintf(logfp, "Error matching regex %s\n", multi_tests[i][0]);
                fprintf(logfp, "Matching input %s falsely reported non-matching\n",
                       multi_tests[i][1]);
                ++ret;
            }

            if (rxvm_search(&compiled, ++end, &start, &end, 0)) {
                if (substring_match(multi_tests[i][3], start, end)) {
                } else {
                    fprintf(logfp, "Error matching regex %s\n", multi_tests[i][0]);
                    fprintf(logfp, "Expecting \"%s\", but substring pointers show \"",
                           multi_tests[i][3]);
                    do {
                        fprintf(logfp, "%c", *start);
                    } while (start++ != end);

                    fprintf(logfp, "\"\n");
                    ++ret;
                }
            } else {
                fprintf(logfp, "Error matching regex %s\n", multi_tests[i][0]);
                fprintf(logfp, "Matching input %s falsely reported non-matching\n",
                       multi_tests[i][1]);
                ++ret;
            }


            rxvm_free(&compiled);
        }

        msg = (ret) ? "FAIL" : "PASS";
        fprintf(trsfp, ":test-result: %s %s #%d\n", msg, __func__, tests);
        printf("%s: %s #%i\n", msg, __func__, i + 1);
    }
}
