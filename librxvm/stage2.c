#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxvm.h"

static void optimise_chain (rxvm_t *cmp, int *chain, int *lookup,
                            int ix)
{
    inst_t inst;
    int end;
    int j;
    int chained;

    chain[0] = ix;
    lookup[ix] = 1;
    chained = 1;
    inst = cmp->exe[ix];

    /* Follow the chain of jmp instructions,
     * saving the index of each of one in 'chain' */
    while (inst.op == OP_JMP) {
        if (cmp->exe[inst.x].op == OP_JMP) {
            chain[chained++] = inst.x;
            lookup[inst.x] = 1;
        }

        inst = cmp->exe[inst.x];
    }

    /* Get the 'x' value of the last saved jmp */
    end = cmp->exe[chain[chained - 1]].x;

    /* Apply the same 'x' value to all saved jmps */
    for (j = 0; j < chained; ++j) {
        cmp->exe[chain[j]].x = end;
    }
}

static int optimise_chains (rxvm_t *cmp)
{
    unsigned int i;
    int *chain;
    int *lookup;
    inst_t inst;

    if ((chain = malloc(sizeof(int) * cmp->size * 2)) == NULL) {
        return RXVM_EMEM;
    }

    memset(chain, 0, sizeof(int) * cmp->size * 2);
    lookup = chain + cmp->size;

    for (i = 0; i < (cmp->size - 1); ++i) {
        inst = cmp->exe[i];

        /* If this instruction is the start of a jmp chain ... */
        if (inst.op == OP_JMP &&
                cmp->exe[inst.x].op == OP_JMP && !lookup[i]) {
            /* optimise it. */
            optimise_chain(cmp, chain, lookup, i);
        }
    }

    free(chain);
    return 0;
}

int stage2 (ir_stack_t *ir, rxvm_t *ret)
{
    stackitem_t *item;
    stackitem_t *next;
    inst_t *inst;
    int i, err;

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
        memcpy(&ret->exe[i], inst, sizeof(inst_t));

        free(item->data);
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
