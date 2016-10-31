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

#include <stdlib.h>
#include <string.h>
#include "rxvm_err.h"
#include "rxvm_common.h"
#include "string_builder.h"
#include "stack.h"
#include "vmcode.h"
#include "stage1.h"
#include "setop.h"

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

static int stack_dupe_from_item (stack_t *stack1, stackitem_t *stop,
                                  stackitem_t *i)
{
    char *temp;
    int ccslen;
    inst_t inst;

    while(1) {
        memcpy(&inst, i->data, sizeof(inst_t));

        /* "class" instructions will require
         * extra work to copy the class string */
        if (inst.op == OP_CLASS) {
            ccslen = strlen(inst.ccs) + 1;
            temp = inst.ccs;
            if ((inst.ccs = malloc(sizeof(char) * ccslen)) == NULL) {
                return RXVM_EMEM;
            }
            memcpy(inst.ccs, temp, sizeof(char) * ccslen);
        }

        if (stack_add_inst_head(stack1, &inst) == NULL) {
            return RXVM_EMEM;
        }

        if (i == stop) break;
        i = i->previous;
    }

    return 0;
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
    int end;
    int j;
    int temp;

    if (rep_n == rep_m) {
        return code_rep_n(cp, rep_n, i);
    } else if (rep_n > rep_m) {
        temp = rep_n;
        rep_n = rep_m;
        rep_m = temp;
    }

    for (j = 0; j < rep_n; ++j) {
        if (stack_dupe_from_item(cp->target, cp->buf->head, i) < 0) {
            return RXVM_EMEM;
        }
    }

    for (j = 0; j < (rep_m - rep_n); ++j) {
        end = (size + 1) * ((rep_m - rep_n) - j);
        set_op_branch(&inst, 1, end);
        if (stack_add_inst_head(cp->target, &inst) == NULL) {
            return RXVM_EMEM;
        }

        if (j == ((rep_m - rep_n) - 1)) {
            stack_cat_from_item(cp->target, cp->buf->head, i);
        } else {
            if (stack_dupe_from_item(cp->target, cp->buf->head, i) < 0) {
                return RXVM_EMEM;
            }
        }
    }

    return 0;
}


int code_rep_more (context_t *cp, int rep_n, unsigned int size, stackitem_t *i)
{
    inst_t inst;
    int j;

    for (j = 0; j < (rep_n - 1); ++j) {
        if (stack_dupe_from_item(cp->target, cp->buf->head, i) < 0) {
            return RXVM_EMEM;
        }
    }

    stack_cat_from_item(cp->target, cp->buf->head, i);

    set_op_branch(&inst, -(size), 1);
    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RXVM_EMEM;
    }

    return 0;
}

int code_rep_less (context_t *cp, int rep_m, unsigned int size, stackitem_t *i)
{
    inst_t inst;
    int end;
    int j;

    for (j = 0; j < rep_m; ++j) {
        end = (size + 1) * (rep_m - j);
        set_op_branch(&inst, 1, end);
        if (stack_add_inst_head(cp->target, &inst) == NULL) {
            return RXVM_EMEM;
        }

        if (j == (rep_m - 1)) {
            stack_cat_from_item(cp->target, cp->buf->head, i);
        } else {
            if (stack_dupe_from_item(cp->target, cp->buf->head, i) < 0) {
                return RXVM_EMEM;
            }
        }
    }

    return 0;
}

int code_rep_n (context_t *cp, int rep_n, stackitem_t *i)
{
    int j;

    for (j = 0; j < (rep_n - 1); ++j) {
        if (stack_dupe_from_item(cp->target, cp->buf->head, i) < 0) {
            return RXVM_EMEM;
        }
    }

    stack_cat_from_item(cp->target, cp->buf->head, i);

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
        return RXVM_EMEM;
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
        return RXVM_EMEM;
    }

    stack_cat_from_item(cp->target, cp->buf->head, i);
    set_op_branch(&inst, -(size), 1);
    if (stack_add_inst_head(cp->target, &inst) == NULL) {
        return RXVM_EMEM;
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
        return RXVM_EMEM;
    }

    stack_cat_from_item(cp->target, cp->buf->head, i);
    return 0;
}

int code_alt (context_t *cp, stackitem_t *i)
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
        return RXVM_EMEM;
    }

    set_op_jmp(&inst, 0);
    cp->target->dangling_alt = stack_add_inst_head(cp->target, &inst);
    if (cp->target->dangling_alt == NULL) {
        return RXVM_EMEM;
    }

    cp->target->dsize = cp->target->size;
    return 0;
}

int code_ccs (context_t *cp)
{
    stack_t *topmost;
    inst_t inst;

    strb_addc(&cp->strb, '\0');
    set_op_class(&inst, cp->strb.buf);

    topmost = (stack_t *)cp->parens->tail->data;
    cp->operand = stack_add_inst_head(topmost, &inst);

    if (cp->operand == NULL) {
        return RXVM_EMEM;
    }

    return 0;
}

int code_match (context_t *cp)
{
    inst_t inst;

    set_op_match(&inst);
    if (stack_add_inst_head(cp->prog, &inst) == NULL) {
        return RXVM_EMEM;
    }

    return 0;
}
