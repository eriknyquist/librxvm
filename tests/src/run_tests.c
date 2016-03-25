#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "test_modules.h"

#define NUM_MODS          3

static const testmod_t mods[NUM_MODS] = {
    test_regexvm_compile, test_regexvm, test_regexvm_err
};

int main (void)
{
    results_t results;
    testmod_t module;
    int i;
	int ret;

    results.passed = 0;
    results.failed = 0;
    ret = 0;

    /* Run all test modules */
    for (i = 0; i < NUM_MODS; ++i) {
        module = mods[i];
        ret += (*module)(&results);
    }

    /* Print results */
    printf("%u tests in total\n", results.passed + results.failed);
    if (!results.passed) {
        printf("All failed\n");
    } else if (!results.failed) {
        printf("All passed\n");
    } else {
        printf("%u passed, %u failed\n", results.passed, results.failed);
    }

	return ret;
}
