#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"
#include "test_modules.h"

//#define NUM_MODS          7
#define NUM_MODS          6

const testmod_t mods[NUM_MODS] = {
    test_rxvm_err, test_rxvm_match, test_rxvm_search,
    test_rxvm_search_nomatch, test_rxvm_compile, test_fuzz_rxvm_match,
    //test_fuzz_full_rxvm_match
};

int main (void)
{
    testmod_t module;
    int i;
    int ret;
    int count;

    count = 1;
    ret = 0;

    printf("1..%d\n", NUM_TESTS);
    /* Run all test modules */
    for (i = 0; i < NUM_MODS; ++i) {
        module = mods[i];
        ret += (*module)(&count);
    }

	return ret;
}
