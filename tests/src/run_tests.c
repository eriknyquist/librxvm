#include <stdio.h>
#include <stdlib.h>
#include "test_regexvm_compile.h"

int main (void)
{
	int ret;

    if ((ret = test_regexvm_compile()) == 0) {
        printf("Test run passed.\n");
    } else {
        printf("Test run failed.\n");
    }

	return ret;
}
