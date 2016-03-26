/*
 * The MIT License (MIT)
 * Copyright (c) 2016 Erik K. Nyquist
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"
#include "stage1.h"
#include "stage2.h"
#include "stack.h"
#include "vm.h"

int regexvm_compile (regexvm_t *compiled, char *exp)
{
    stack_t *ir;
    int ret;

    if ((ret = stage1(exp, &ir)) < 0)
        return ret;

    if ((ret = stage2(ir, compiled)) < 0)
        return ret;

    return 0;
}

int regexvm_iter (regexvm_t *compiled, char *input, char **start, char **end)
{
    threads_t tm;
    int ret;

    if ((ret = vm_init(&tm, compiled->size)) != 0)
        goto cleanup;

    ret = 0;
    while (vm_execute(&tm, compiled, &input) && tm.match_end == NULL);

    if (tm.match_end == NULL) {
        if (start)
            *start = NULL;
        if (end)
            *end = NULL;
    } else {
        if (start)
            *start = tm.match_start;
        if (end)
            *end = tm.match_end;
        ret = 1;
    }

cleanup:
    vm_free(&tm);
    return ret;
}

int regexvm_match (regexvm_t *compiled, char *input)
{
    threads_t tm;
    int ret;

    if ((ret = vm_init(&tm, compiled->size)) != 0)
        goto cleanup;

    if (vm_execute(&tm, compiled, &input))
        goto cleanup;

    if (tm.match_end == (input - 1))
        ret = 1;

cleanup:
    vm_free(&tm);
    return ret;
}

void regexvm_print (regexvm_t *compiled)
{
    unsigned int i;
    inst_t *inst;

    for (i = 0; i < compiled->size; i++) {
        inst = compiled->exe[i];

        switch(inst->op) {
            case OP_CHAR:
                printf("%d\tchar %c\n", i, inst->c);
            break;
            case OP_ANY:
                printf("%d\tany\n", i);
            break;
            case OP_CLASS:
                printf("%d\tclass %s\n", i, inst->ccs);
            break;
            case OP_BRANCH:
                printf("%d\tbranch %d %d\n", i, inst->x, inst->y);
            break;
            case OP_JMP:
                printf("%d\tjmp %d\n", i, inst->x);
            break;
            case OP_MATCH:
                printf("%d\tmatch\n", i);
            break;
        }
    }
}

void regexvm_free (regexvm_t *compiled)
{
    unsigned int i;
    inst_t *inst;

    for (i = 0; i < compiled->size; i++) {
        inst = compiled->exe[i];

        if (inst->ccs != NULL)
            free(inst->ccs);

        free(inst);
    }

    free(compiled->exe);
}
