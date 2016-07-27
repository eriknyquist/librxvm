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
#include <string.h>
#include "rxvm.h"

static void optimise_chain (rxvm_t *cmp, int *chain, int *lookup,
                            int ix)
{
    inst_t *inst;
    int end;
    int j;
    int chained;

    chain[0] = ix;
    lookup[ix] = 1;
    chained = 1;
    inst = cmp->exe[ix];

    /* Follow the chain of jmp instructions,
     * saving the index of each of one in 'chain' */
    while (inst->op == OP_JMP) {
        if (cmp->exe[inst->x]->op == OP_JMP) {
            chain[chained++] = inst->x;
            lookup[inst->x] = 1;
        }

        inst = cmp->exe[inst->x];
    }

    /* Get the 'x' value of the last saved jmp */
    end = cmp->exe[chain[chained - 1]]->x;

    /* Apply the same 'x' value to all saved jmps */
    for (j = 0; j < chained; ++j) {
        cmp->exe[chain[j]]->x = end;
    }
}

static int optimise_chains (rxvm_t *cmp)
{
    int i;
    int *chain;
    int *lookup;
    inst_t *inst;

    if ((chain = malloc(sizeof(int) * cmp->size)) == NULL) {
        return RVM_EMEM;
    }

    if ((lookup = malloc(sizeof(int) * cmp->size)) == NULL) {
        free(chain);
        return RVM_EMEM;
    }

    memset(chain, 0, sizeof(int) * cmp->size);
    memset(lookup, 0, sizeof(int) * cmp->size);

    for (i = 0; i < (cmp->size - 1); ++i) {
        inst = cmp->exe[i];

        /* If this instruction is the start of a jmp chain ... */
        if (inst->op == OP_JMP &&
                cmp->exe[inst->x]->op == OP_JMP && !lookup[i]) {
            /* optimise it. */
            optimise_chain(cmp, chain, lookup, i);
        }
    }

    free(chain);
    free(lookup);
    return 0;
}

int stage2 (stack_t *ir, rxvm_t *ret)
{
    stackitem_t *item;
    stackitem_t *next;
    inst_t *inst;
    unsigned int i;
    int err;

    /* array to hold pointers to the already-allocated instructions */
    if ((ret->exe = malloc(sizeof(inst_t *) * ir->size)) == NULL)
        return RVM_EMEM;

    err = 0;
    item = ir->tail;
    for (i = 0; i < ir->size; i++) {
        next = item->previous;
        inst = (inst_t *) item->data;

        /* Now that the final length of instructions
         * is known, the x & y pointers of branch and jmp
         * instructions can be updated with valid indices */
        if (inst->op == OP_BRANCH) {
            inst->x += i;
            inst->y += i;
        } else if (inst->op == OP_JMP) {
            inst->x += i;
        }

        /* copy instruction pointer into array
         * and free list item struct. */
        ret->exe[i] = inst;
        free(item);
        item = next;
    }

    ret->size = ir->size;
    ret->simple = NULL;

    if (ir->dsize)
        err = optimise_chains(ret);

    free(ir);
    return err;
}
