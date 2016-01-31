#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "common.h"

#if (DEBUG)
static unsigned int mcnt;
static unsigned int fcnt;
#endif /* DEBUG */

stack_t *create_stack(void)
{
    stack_t *newstack;

    if ((newstack = malloc(sizeof(stack_t))) == NULL) {
        return NULL;
    }

#if (DEBUG)
    mcnt++;
#endif /* DEBUG */
    newstack->head = NULL;
    newstack->tail = NULL;
    newstack->dangling_cat = NULL;
    newstack->size = 0;
    return newstack;
}

stackitem_t *create_item(inst_t *inst)
{
    stackitem_t *item;
    size_t dsize;

    if ((item = malloc(sizeof(stackitem_t))) == NULL)
        return NULL;

    if ((item->inst = malloc(sizeof(inst_t))) == NULL)
        return NULL;

#if (DEBUG)
    mcnt += 2;
#endif /* DEBUG */

    item->inst->op = inst->op;
    item->inst->c = inst->c;
    item->inst->x = inst->x;
    item->inst->y = inst->y;

    if (inst->ccs != NULL) {
        dsize = (sizeof(char) * strlen(inst->ccs)) + 1;
        if ((item->inst->ccs = malloc(dsize)) == NULL)
            return NULL;

#if (DEBUG)
        mcnt++;
#endif /* DEBUG */
        strncpy(item->inst->ccs, inst->ccs, dsize);
    }

    item->next = NULL;
    item->previous = NULL;
    return item;
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

    if (!stack->head) return;
    i = stack->head;
    while (i != NULL) {
        next = i->next;

        if (i->inst->ccs != NULL) {
            free(i->inst->ccs);
#if (DEBUG)
            fcnt++;
#endif /* DEBUG */
        }

        if (i->inst != NULL) {
            free(i->inst);
#if (DEBUG)
            fcnt++;
#endif /* DEBUG */
        }

        free(i);
        i = next;
#if (DEBUG)
        fcnt++;
#endif /* DEBUG */
    }

    free(stack);
#if (DEBUG)
    fcnt++;
#endif /* DEBUG */
}

#if (DEBUG)
void print_alloc_summary (void)
{
    printf("malloc() calls: %u\nfree() calls: %u\n", mcnt, fcnt);
}
#endif /* DEBUG */
