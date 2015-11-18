#ifndef STACK_H_
#define STACK_H_

typedef struct stackitem {
    char *data;
    struct stackitem *next;
    struct stackitem *previous;
} stackitem_t;

typedef struct stack {
    struct stackitem *head;
    struct stackitem *tail;
    unsigned int size;
} stack_t;

stack_t *create_stack (void);
int stack_push (stack_t *stack, const char *data);
const char *stack_pop (stack_t *stack);
void stack_free (stack_t *stack);

#endif
