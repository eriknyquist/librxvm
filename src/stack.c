#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stack.h"

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

stackitem_t *create_item(const char *data)
{
    stackitem_t *item;
    if (!(item = malloc(sizeof(stackitem_t))))
        return NULL;

    item->data = malloc(sizeof(const char) * strlen(data));
    strncpy(item->data, data, sizeof(const char) * strlen(data));
    item->next = NULL;
    return item;
}

int stack_push (stack_t *stack, const char *data)
{
    stackitem_t *new;
    new = create_item(data);
    if (new == NULL) {
        return -1;
    }

    if (stack->size > 0) {
        new->next = stack->head;
    } else {
        stack->tail = new;
    }

    stack->head = new;
    stack->size++;
    return 0;
}

const char *stack_pop (stack_t *stack)
{
    const char *data;

    if (stack->size == 0) {
        return NULL;
    }

    data = stack->head->data;
    stack->head = stack->head->next;
    stack->size--;
    return data;
}

void stack_free (stack_t *stack)
{
    stackitem_t *next;

    if (!stack->head) return;
    while (stack->head) {
        next = stack->head->next;
        free(stack->head->data);
        free(stack->head);
        stack->head = next;
    }

    free(stack);
}
