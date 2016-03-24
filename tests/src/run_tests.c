#include <stdio.h>
#include <stdlib.h>
#include "test_regexvm_compile.h"
#include "test_regexvm_err.h"
#include "test_regexvm.h"

int main (void)
{
    const char *msg;
	int ret;

    ret = 0;
    ret += test_regexvm_compile();
    ret += test_regexvm_err();
    ret += test_regexvm();

    msg = (ret == 0) ? "passed" : "failed";
    printf("Test run %s\n", msg);
	return ret;
}
