#include <stdlib.h>
#include <string.h>
#include "regexvm_err.h"
#include "regexvm_common.h"
#include "stack.h"

/* set_op functions:
 * a bunch of convenience functions for populating
 * inst_t types for all instructions */
static void set_op_class (inst_t *inst, char *ccs)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_CLASS;
    inst->ccs = ccs;
}

static void set_op_branch (inst_t *inst, int x, int y)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_BRANCH;
    inst->x = x;
    inst->y = y;
}

static void set_op_jmp (inst_t *inst, int x)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_JMP;
    inst->x = x;
}

static void set_op_jlt (inst_t *inst, int j, int i)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_JLT;
    inst->x = j;
    inst->y = i;
}

static void set_op_match (inst_t *inst)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_MATCH;
}

/* create_inst: allocate space for an inst_t, populate it
   with the data in 'inst' and return a pointer to it */
static inst_t *create_inst (inst_t *inst)
{
    inst_t *new;

    if ((new = malloc(sizeof(inst_t))) == NULL)
        return NULL;

    memset(new, 0, sizeof(inst_t));

    new->op = inst->op;
    new->c = inst->c;
    new->x = inst->x;
    new->y = inst->y;
    new->ccs = inst->ccs;

    return new;
}

stackitem_t *stack_add_inst_head (stack_t *stack, inst_t *inst)
{
    return stack_add_head(stack, (void *) create_inst(inst));
}

static stackitem_t *stack_add_inst_tail (stack_t *stack, inst_t *inst)
{
    return stack_add_tail(stack, (void *) create_inst(inst));
}

/* stack_cat_from_item: append items from a stack, starting from
 * stack item 'i', until stack item 'stop' is reached, onto stack1. */
static void stack_cat_from_item (stack_t *stack1, stackitem_t *stop,
                                 stackitem_t *i)
{
    while (1) {
        stack_point_new_head(stack1, i);
        if (i == stop) break;
        i = i->previous;
    }
}

/* when an alternation operator "|" is seen, it leaves a 'jmp'
 * instructuction with an unset pointer. This pointer marks the end of
 * that alternation, and attach_dangling_alt is called to set this
 * pointer when 1) another "| alt. token is seen, 2) a right-paren.
 * token is seen, or 3) the end of the input string is reached. */
void attach_dangling_alt (context_t *cp)
{
    inst_t *inst;

    if (cp->target->dangling_alt == NULL) {
        return;
    } else {
        inst = (inst_t *) cp->target->dangling_alt->data;
        inst->x = (cp->target->size - cp->target->dsize) + 1;

        cp->target->dangling_alt = NULL;
    }
}

int code_rep_range (context_t *cp, int rep_n, int rep_m, unsigned int size,
                    stackitem_t *i)
{
    inst_t inst;

    set_op_jlt(&inst, 2, rep_n);
    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    set_op_branch(&inst, 1, size + 2);
    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    stack_cat_from_item(cp->target, cp->buf->head, i);

    set_op_jlt(&inst, -(size + 2), rep_m - 1);
    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    return 0;
}


int code_rep_more (context_t *cp, int rep_n, unsigned int size, stackitem_t *i)
{
    inst_t inst;

    stack_cat_from_item(cp->target, cp->buf->head, i);

    set_op_jlt(&inst, -(size), rep_n - 1);
    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    set_op_branch(&inst, -(size + 1), 1);
    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    return 0;
}

int code_rep_less (context_t *cp, int rep_m, unsigned int size, stackitem_t *i)
{
    inst_t inst;

    set_op_branch(&inst, 1, size + 2);
    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    stack_cat_from_item(cp->target, cp->buf->head, i);

    set_op_jlt(&inst, -(size + 1), rep_m - 1);
    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    return 0;
}

int code_rep_n (context_t *cp, int rep_n, unsigned int size, stackitem_t *i)
{
    inst_t inst;

    stack_cat_from_item(cp->target, cp->buf->head, i);

    set_op_jlt(&inst, -(size), rep_n + 1);
    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    return 0;
}

int code_one (context_t *cp, unsigned int size, stackitem_t *i)
{
    inst_t inst;
    /* x = current position MINUS size of operand buf
     * y = current position PLUS 1 */
    set_op_branch(&inst, -(size), 1);
    stack_cat_from_item(cp->target, cp->buf->head, i);

    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    return 0;
}

int code_zero (context_t *cp, unsigned int size, stackitem_t *i)
{
    inst_t inst;

    /* x = current position PLUS 1
     * y = current position PLUS size of operand buf PLUS 2 */
    set_op_branch(&inst, 1, size + 2);
    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    stack_cat_from_item(cp->target, cp->buf->head, i);
    set_op_branch(&inst, -(size), 1);
    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    return 0;
}

int code_onezero (context_t *cp, unsigned int size, stackitem_t *i)
{
    inst_t inst;

    /* x = current position PLUS 1
     * y = current position PLUS size of operand buf PLUS 1 */
    set_op_branch(&inst, 1, size + 1);

    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    stack_cat_from_item(cp->target, cp->buf->head, i);
    return 0;
}

int code_alt (context_t *cp, unsigned int size, stackitem_t *i)
{
    inst_t inst;

    if (i != NULL) {
        stack_cat_from_item(cp->target, cp->buf->head, i);
    }

    if (cp->target->dsize && !cp->chained) {
                cp->chained = 1;
    }

    attach_dangling_alt(cp);

    /* x = current position PLUS 1
     * y = current position PLUS size of target buf PLUS 2 */
    set_op_branch(&inst, 1, cp->target->size + 2);

    if (stack_add_inst_tail(cp->target, &inst) == NULL) {
        return RVM_EMEM;
    }

    set_op_jmp(&inst, 0);
    cp->target->dangling_alt = stack_add_inst_head(cp->target, &inst);
    if (cp->target->dangling_alt == NULL) {
        return RVM_EMEM;
    }

    cp->target->dsize = cp->target->size;
    return 0;
}

int code_ccs (context_t *cp)
{
    inst_t inst;

    cp->ccs[cp->clen] = '\0';
    set_op_class(&inst, cp->ccs);
    cp->operand =
                stack_add_inst_head((stack_t *) cp->parens->tail->data, &inst);
    if (cp->operand == NULL) {
        return RVM_EMEM;
    }

    cp->ccs = NULL;
    cp->clen = 0;

    return 0;
}

int code_match (context_t *cp)
{
    inst_t inst;

    set_op_match(&inst);
    if (stack_add_inst_head(cp->prog, &inst) == NULL) {
        return RVM_EMEM;
    }

    return 0;
}
