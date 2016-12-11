#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "rxvm.h"
#include "test_common.h"

#define NUMNAMES        7

unsigned int rand_range (unsigned int low, unsigned int high)
{
    return (unsigned int) low + (rand() % ((high - low) + 1));
}

#if (TEST_PRINT_SIZES)
static void print_heap_usage (unsigned int size)
{
    size_t progsize;
    size_t vmsize;

    progsize = sizeof(rxvm_t) + (sizeof(inst_t) * size);
    vmsize = ((sizeof(int) * size) * 2) + (size * 2);
    printf("compiled size: %zu bytes\nVM size: %zu bytes\n", progsize, vmsize);
}
#endif

int compile_testexp (rxvm_t *compiled, char *exp)
{
    int ret;

    if ((ret = rxvm_compile(compiled, exp)) < 0)
        return ret;

#if (TEST_PRINT_SIZES)
    printf("expression: %s\n", exp);
    print_heap_usage(compiled->size);
    printf("\n");
#endif
    return 0;
}

void test_err (char *regex, char *input, const char *test, char *msg, int ret)
{
    fprintf(logfp, "          test set : %s\n", test);
    fprintf(logfp, "reason for failure : %s\n", msg);
    fprintf(logfp, "         exit code : %d\n", ret);
    fprintf(logfp, "        expression : %s\n", regex);
    fprintf(logfp, "        input text : %s\n\n", input);
}

char *hrsize (uint64_t size)
{
    static const char *names[NUMNAMES] =
    {
        "EB", "PB", "TB", "GB", "MB", "KB", "B"
    };
    char *ret;
    uint64_t mult;
    size_t retsize;
    int i;

    retsize = sizeof(char) * 20;
    mult = EXABYTES;
    ret = malloc(retsize);

    for (i = 0; i < NUMNAMES; ++i, mult /= 1024) {
        if (size < mult)
            continue;

        if ((size % mult) == 0) {
            snprintf(ret, retsize, "%"PRIu64" %s", size / mult, names[i]);
        } else {
            snprintf(ret, retsize, "%.2f %s", (float) size / mult, names[i]);
        }

        return ret;
    }

    strcpy(ret, "0");
    return ret;
}
