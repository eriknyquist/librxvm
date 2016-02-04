#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "stack.h"
#include "stage1.h"

int main (int argc, char *argv[])
{
    stack_t *prog;
    int ret;

    if (argc != 2) {
        printf("Usage: %s <regex>\n", argv[0]);
        exit(1);
    }

    ret = stage1(argv[1], &prog);

    printf("return code: %d\n", ret);
    if (ret < 0) exit(ret);

    printf("Input expression: %s\n", argv[1]);
    printf("after stage 1 compilation:\n\n");
    print_prog(prog);
    printf("size: %u\n", prog->size);

    stack_free(prog);
    return 0;
}
