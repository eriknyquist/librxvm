#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "test_modules.h"

#define NUM_MODS          4

const testmod_t mods[NUM_MODS] = {
    test_regexvm_compile, test_regexvm_match, test_regexvm_iter,
    test_regexvm_err
};

int main (void)
{
    results_t results;
    testmod_t module;
    int i;

    results.passed = 0;
    results.failed = 0;

    /* Run all test modules */
    for (i = 0; i < NUM_MODS; ++i) {
        module = mods[i];
        (*module)(&results);
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

	return results.failed;
}
