#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"
#include "test_modules.h"

FILE *logfp;
FILE *trsfp;

int main (void)
{
    if ((trsfp = fopen(TEST_TRS, "w")) == NULL) {
        printf("Unable to open file "TEST_TRS" for writing\n");
        return 1;
    }

    if ((logfp = fopen(TEST_LOG, "w")) == NULL) {
        printf("Unable to open file "TEST_LOG" for writing\n");
        return 1;
    }

    test_rxvm_compile();
    test_rxvm_err();
    test_rxvm_api();
    test_rxvm_match();
    test_rxvm_search();
    test_rxvm_search_multi();
    test_rxvm_search_nomatch();
    test_fuzz_rxvm_match();
    //test_fuzz_full_rxvm_match();
    test_rxvm_lfix_heuristic();
    test_rxvm_lfix_to_str();

    fclose(logfp);
    fclose(trsfp);
	return 0;
}
