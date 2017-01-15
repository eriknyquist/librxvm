#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"
#include "test_modules.h"

//#define NUM_MODS          9
#define NUM_MODS          8

FILE *logfp;
FILE *trsfp;

const testmod_t mods[NUM_MODS] = {
    test_rxvm_compile, test_rxvm_lfix_heuristic, test_rxvm_err, test_rxvm_match,
    test_rxvm_search, test_rxvm_search_multi, test_rxvm_search_nomatch,
    test_fuzz_rxvm_match, //test_fuzz_full_rxvm_match
};

int main (void)
{
    testmod_t module;
    int i;
    int ret;
    int count;

    count = 1;
    ret = 0;

    if ((trsfp = fopen(TEST_TRS, "w")) == NULL) {
        printf("Unable to open file "TEST_TRS" for writing\n");
        return 1;
    }

    if ((logfp = fopen(TEST_LOG, "w")) == NULL) {
        printf("Unable to open file "TEST_LOG" for writing\n");
        return 1;
    }

    /* Run all test modules */
    for (i = 0; i < NUM_MODS; ++i) {
        module = mods[i];
        ret += (*module)(&count);
    }

    fclose(logfp);
    fclose(trsfp);
	return 0;
}
