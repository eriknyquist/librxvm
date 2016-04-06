#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"
#include "test_common.h"
#include "test_modules.h"

#define NUM_MODS          5

const testmod_t mods[NUM_MODS] = {
    test_regexvm_err, test_regexvm_match, test_regexvm_iter,
    test_regexvm_compile, test_fuzz_regexvm_match
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
