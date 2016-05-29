#ifndef _VMCODE_H_
#define _VMCODE_H_

stackitem_t *stack_add_inst_head (stack_t *stack, inst_t *inst);
void attach_dangling_alt (context_t *cp);

int code_match (context_t *cp);
int code_ccs (context_t *cp);
int code_one (context_t *cp, unsigned int size, stackitem_t *i);
int code_zero (context_t *cp, unsigned int size, stackitem_t *i);
int code_onezero (context_t *cp, unsigned int size, stackitem_t *i);
int code_alt (context_t *cp, unsigned int size, stackitem_t *i);
int code_rep_n (context_t *cp, int rep_n, unsigned int size, stackitem_t *i);
int code_rep_less (context_t *cp, int rep_m, unsigned int size, stackitem_t *i);
int code_rep_more (context_t *cp, int rep_n, unsigned int size, stackitem_t *i);
int code_rep_range (context_t *cp, int rep_n, int rep_m, unsigned int size,
                    stackitem_t *i);

#endif
