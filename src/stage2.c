#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"

int stage2 (stack_t *ir, regexvm_t *ret)
{
    stackitem_t *item;
    stackitem_t *next;
    unsigned int i;

    if ((ret->exe = malloc(sizeof(stack_t *) * ir->size)) == NULL)
        return RVM_EMEM;

    item = ir->tail;
    for (i = 0; i < ir->size; i++) {
        next = item->previous;
        if (item->inst->op == OP_BRANCH) {
            item->inst->x += i;
            item->inst->y += i;
        }

        if (item->inst->op == OP_JMP) {
            item->inst->x += i;
        }

        ret->exe[i] = item->inst;
        free(item);
        item = next;
    }

    ret->size = ir->size;
    free(ir);
    return 0;
}
