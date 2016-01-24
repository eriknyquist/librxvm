#ifndef STACK_H_
#define STACK_H_

stack_t *create_stack (void);
stackitem_t *stack_add_head (stack_t *stack, inst_t *inst);
stackitem_t *stack_add_tail (stack_t *stack, inst_t *inst);
void stack_cat (stack_t **stack1, stack_t **stack2);
void stack_free (stack_t *stack);

#endif
