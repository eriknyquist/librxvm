#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"

char *iter_tests[NUM_TESTS_ITER][3] =
{
    {"x+", "oki9u08y72389)*Y9*YO(U*IOxxxxokKIIjJoiOKjImIOKmOIOuyyRKj", "xxxx"},
    {"x+", "oki9u08y72389)*Y9*YO(U*IOxokKIIjJoiOKjImIOKmbggbggOIOuyyRKj", "x"},
    {"aa*b+c?", "09t5ol55llll5taaaaabbb0pppo0yiy5yi0pyi459tim5om", "aaaaabbb"},
    {"aa*b+c?", "09taaa5aoaaal55laaal5tabc0pppo0yiy5yi0pyi459tim5om", "abc"},
    {"ab(cd(ef)?)*|qtqt|ux*", "OPIPkIOIkpOI(*Ijoiab998779op90ii90op90", "ab"},
    {"ab(cd(ef)?)*|qtqt|ux*", "OPIPkIOuIkpOI(*Ijoiab998779op90ii90op90", "u"},
    {"ab(cd(ef)?)*|qtqt|ux*", "OPIPkIOuIkp78(&-00-0=*78989io", "u"},
    {"ab(cd(ef)?)*|qtqt|ux*", "poihfskjlpfjojsefjw098hqtqtpoefhisr", "qtqt"},
    {"ab(cd(ef)?)*|qtqt|ux*", "OPIPkIa99abcdefcdcd8779op90ii90", "abcdefcdcd"},
    {"xy(ab(cd(ef(gh)*)*)*)*", "_+)(*%&%$xyabcdefgh0989mkmkmk", "xyabcdefgh"},
    {"xy(ab(cd(ef(gh)*)*)*)*", "aqtytqaoiquxyabababab_=__)+&*6", "xyabababab"},
    {"xy(ab(cd(ef(gh)*)*)*)*", "@|@||@{||}@xyabcdefef_=__)+&*6", "xyabcdefef"},
    {"([0-9]\\.?)+", "kopiNmKM<L.,loIMOi192.168.12.33JUIOIO", "192.168.12.33"},
    {"([0-9]\\.?)+", "kopiNmKM<L.,loIMOiiiiiiiiiiiALPlapALPA1JUIOIO", "1"},
    {"([0-9]\\.?)+", "KDJIWDoieOFIWOIF66.78EJofejEFJAFIEAjDMKAN", "66.78"},
    {"word|xy(regex([Vv][Mm]|stuff)?)*", "869ioKioword*&guijiIOJiou", "word"},
    {"word|xy(regex([Vv][Mm]|stuff)?)*",
        "869i83498l3ko.........KioxyregexVMjiIOJiou", "xyregexVM"},
    {"word|xy(regex([Vv][Mm]|stuff)?)*", "869ioKioxy*&guijiIOJiou", "xy"},
    {"word|xy(regex([Vv][Mm]|stuff)?)*",
        "869ioKiopskoJh9q89j98djxyregexVMregexregexstuff*&guijiIOJiou",
        "xyregexVMregexregexstuff"}
};

static int substring_match (char *string, char *start, char *end)
{
    while (*string) {
        if (*(string++) != *(start++))
            return 0;
    }

    return (start == end) ? 1 : 0;
}

int test_rxvm_search (int *count)
{
    rxvm_t compiled;
    const char *msg;
    char *start;
    char *end;
    int ret;
    int err;
    int i;

    for (i = 0; i < NUM_TESTS_ITER; ++i) {
        ret = 0;
        if ((err = compile_testexp(&compiled, iter_tests[i][0])) < 0) {
            fprintf(logfp, "Error compiling regex %s\n", iter_tests[i][0]);
            ++ret;
        } else {
            if (rxvm_search(&compiled, iter_tests[i][1], &start, &end, 0)) {
                if (substring_match(iter_tests[i][2], start, end)) {
                } else {
                    fprintf(logfp, "Error matching regex %s\n", iter_tests[i][0]);
                    fprintf(logfp, "Expecting \"%s\", but substring pointers show \"",
                           iter_tests[i][2]);
                    while (start != end) {
                        fprintf(logfp, "%c", *start);
                        ++start;
                    }
                    fprintf(logfp, "\"\n");
                    ++ret;
                }
            } else {
                fprintf(logfp, "Error matching regex %s\n", iter_tests[i][0]);
                fprintf(logfp, "Matching input %s falsely reported non-matching\n",
                       iter_tests[i][1]);
                ++ret;
            }

            rxvm_free(&compiled);
        }

        msg = (ret) ? "FAIL" : "PASS";
        fprintf(trsfp, ":test-result: %s %s #%d\n", msg, __func__, *count);
        ++(*count);
    }

    return ret;
}
