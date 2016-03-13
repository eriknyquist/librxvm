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
#include <sys/types.h>
#include <sys/stat.h>
#include "regexvm_common.h"

static stackitem_t *create_item(inst_t *inst)
{
    stackitem_t *item;
    size_t dsize;

    if ((item = malloc(sizeof(stackitem_t))) == NULL)
        return NULL;

    if ((item->inst = malloc(sizeof(inst_t))) == NULL)
        return NULL;

    memset(item->inst, 0, sizeof(inst_t));

    item->inst->op = inst->op;
    item->inst->c = inst->c;
    item->inst->x = inst->x;
    item->inst->y = inst->y;

    if (inst->ccs != NULL) {
        dsize = (sizeof(char) * strlen(inst->ccs)) + 1;
        if ((item->inst->ccs = malloc(dsize)) == NULL)
            return NULL;

        strncpy(item->inst->ccs, inst->ccs, dsize);
    }

    item->next = NULL;
    item->previous = NULL;
    return item;
}

stack_t *create_stack(void)
{
    stack_t *newstack;

    if ((newstack = malloc(sizeof(stack_t))) == NULL) {
        return NULL;
    }

    newstack->head = NULL;
    newstack->tail = NULL;
    newstack->dangling_alt = NULL;
    newstack->size = 0;
    return newstack;
}

void stack_point_new_head(stack_t *stack, stackitem_t *new)
{
    if (stack->head == NULL) {
        stack->tail = new;
    } else {
        stack->head->previous = new;
        new->next = stack->head;
    }

    stack->size++;
    stack->head = new;
}

stackitem_t *stack_add_head (stack_t *stack, inst_t *inst)
{
    stackitem_t *new;
    new = create_item(inst);
    if (new == NULL) {
        return NULL;
    }

    stack_point_new_head(stack, new);
    return stack->head;
}

stackitem_t *stack_add_tail (stack_t *stack, inst_t *inst)
{
    stackitem_t *new;
    new = create_item(inst);
    if (new == NULL) {
        return NULL;
    }

    if (stack->tail == NULL) {
        stack->head = new;
    } else {
        stack->tail->next = new;
        new->previous = stack->tail;
    }

    stack->tail = new;
    stack->size++;
    return new;
}

void stack_cat (stack_t *stack1, stack_t *stack2)
{
    if (stack1 != NULL && stack1->head == NULL) {
        stack1->head = stack2->head;
        stack1->tail = stack2->tail;
        stack1->size = stack2->size;
    } else if (stack2 != NULL && stack2->head != NULL) {
        stack1->head->previous = stack2->tail;
        stack2->tail->next = stack1->head;
        stack1->head = stack2->head;
        stack1->size += stack2->size;
    }
}

void stack_free (stack_t *stack)
{
    stackitem_t *i;
    stackitem_t *next;

    if (!stack) return;
    i = stack->head;
    while (i != NULL) {
        next = i->next;

        if (i->inst->ccs != NULL) {
            free(i->inst->ccs);
        }

        if (i->inst != NULL) {
            free(i->inst);
        }

        free(i);
        i = next;
    }

    free(stack);
}
