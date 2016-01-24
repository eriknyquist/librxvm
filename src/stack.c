#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "common.h"

stack_t *create_stack(void)
{
    stack_t *newstack = malloc(sizeof(stack_t));
    if (!newstack) {
        return NULL;
    }

    newstack->head = NULL;
    newstack->tail = NULL;
    newstack->size = 0;
    return newstack;
}

stackitem_t *create_item(inst_t *inst)
{
    stackitem_t *item;
    size_t dsize;

    if (!(item = malloc(sizeof(stackitem_t))))
        return NULL;

    item->inst = malloc(sizeof(inst_t));
    item->inst->op = inst->op;
    item->inst->c = inst->c;
    item->inst->x = inst->x;
    item->inst->y = inst->y;

    if (inst->ccs) {
        dsize = ((sizeof(char) * strlen(inst->ccs))) + 1;
        item->inst->ccs = malloc(dsize);
        strncpy(item->inst->ccs, inst->ccs, dsize);
    }

    item->next = NULL;
    item->previous = NULL;
    return item;
}

stackitem_t *stack_add_head (stack_t *stack, inst_t *inst)
{
    stackitem_t *new;
    new = create_item(inst);
    if (new == NULL) {
        return NULL;
    }

    if (stack->head == NULL) {
        stack->tail = new;
    } else {
        stack->head->previous = new;
        new->next = stack->head;
    }

    stack->head = new;
    return new;
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
    return new;
}

void stack_cat (stack_t **stack1, stack_t **stack2)
{
    if (*stack1 == NULL || (*stack1)->head == NULL) {
        *stack1 = *stack2;
    } else if (*stack2 != NULL || (*stack2)->head != NULL) {
        (*stack1)->tail->next = (*stack2)->head;
        (*stack2)->head->previous = (*stack1)->tail;
        (*stack1)->tail = (*stack2)->tail;
    }

    (*stack2)->head = NULL;
    (*stack2)->tail = NULL;
    (*stack2) = NULL;
}

void stack_free (stack_t *stack)
{
    stackitem_t *next;

    if (!stack->head) return;
    while (stack->head != NULL) {
        next = stack->head->next;
        free(stack->head->inst->ccs);
        free(stack->head->inst);
        free(stack->head);
        stack->head = next;
    }

    free(stack);
}
