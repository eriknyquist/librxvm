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

int stage2 (stack_t *ir, regexvm_t *ret)
{
    stackitem_t *item;
    stackitem_t *next;
    unsigned int i;

    /* array to hold pointers to the already-allocated instructions */
    if ((ret->exe = malloc(sizeof(inst_t *) * ir->size)) == NULL)
        return RVM_EMEM;

    item = ir->tail;
    for (i = 0; i < ir->size; i++) {
        next = item->previous;

        /* Now that the final length of instructions
         * is known, the x & y pointers of branch and jmp
         * instructions can be updated with valid indices */
        if (item->inst->op == OP_BRANCH) {
            item->inst->x += i;
            item->inst->y += i;
        } else if (item->inst->op == OP_JMP) {
            item->inst->x += i;
        }

        /* copy instruction pointer into array
         * and free list item struct. */
        ret->exe[i] = item->inst;
        free(item);
        item = next;
    }

    ret->size = ir->size;
    free(ir);
    return 0;
}
