#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lex.h"
#include "err.h"
#include "common.h"
#include "stack.h"

#define ISOP(x) \
((x == ONE || x == ZERO || x == ONEZERO || x == CONCAT) ? 1 : 0)

/* pointers for the start & end of text in
 * input string for the current token */
extern char *lp1;
extern char *lpn;

enum {STATE_START, STATE_CHARC};

/* pretty-print an instruction (debug) */
void print_inst (inst_t *inst, int num)
{
    unsigned int i;
    size_t n;

    switch (inst->op) {
        case OP_CHAR:
            printf("%d\tchar %c", num, inst->c);
        break;

        case OP_ANY:
            printf("%d\tany", num);
        break;

        case OP_CLASS:
            n = strlen(inst->ccs);
            printf("%d\tclass ", num);
            for (i = 0; i < n; i++)
                printf("%c", inst->ccs[i]);
        break;

        case OP_BRANCH:
            printf("%d\tbranch x x", num);
        break;

        case OP_JMP:
            printf("%d\tjmp x", num);
        break;

        case OP_MATCH:
            printf("%d\tmatch", num);
    }
}

/* pretty-print a list of instructions (debug) */
void print_prog (stack_t *stack)
{
    int num = 0;
    stackitem_t *item;
    inst_t *inst;

    if (stack == NULL) return;

    item = stack->tail;
    while (item != NULL) {
        inst = item->inst;
        print_inst(inst, num++);
        if (item == stack->tail) {
            printf(" (tail)\n");
        } else if (item == stack->head) {
            printf(" (head)\n");
        } else {
            printf("\n");
        }

        item = item->previous;
    }
}

/* set_op functions:
 * a bunch of convenience functions for populating
 * inst_t types for all instructions */
void set_op_char (inst_t *inst, char c)
{
    inst->op = OP_CHAR;
    inst->c = c;
    inst->x = NULL;
    inst->y = NULL;
    inst->ccs = NULL;
}

void set_op_class (inst_t *inst, char *ccs)
{
    inst->op = OP_CLASS;
    inst->ccs = ccs;
    inst->x = NULL;
    inst->y = NULL;
}

void set_op_any (inst_t *inst)
{
    inst->op = OP_ANY;
    inst->x = NULL;
    inst->y = NULL;
    inst->ccs = NULL;
}

void set_op_branch (inst_t *inst, stackitem_t *x, stackitem_t *y)
{
    inst->op = OP_BRANCH;
    inst->x = x;
    inst->y = y;
    inst->ccs = NULL;
}

void set_op_jmp (inst_t *inst, stackitem_t *x)
{
    inst->op = OP_JMP;
    inst->x = x;
    inst->y = NULL;
    inst->ccs = NULL;
}

void set_op_match (inst_t *inst)
{
    inst->op = OP_MATCH;
    inst->x = NULL;
    inst->y = NULL;
    inst->ccs = NULL;
}

/* stack_cat_from_item: appends items from a stack, starting from
 * stack item 'i', until stack item 'stop' is reached, onto stack1.*/
void stack_cat_from_item(stack_t *stack1, stackitem_t *stop, stackitem_t *i)
{
    while (1) {
        stack_point_new_head(stack1, i);
        if (i == stop) break;
        i = i->previous;
    }
}

/* process_op: processes operator 'tok' against a buffer of
 * literals 'buf', where the first operand is 'item' and the
 * output is appended to 'prog' */
int process_op (stack_t *prog, stackitem_t *item, stack_t *buf, int tok)
{
    stackitem_t *i;
    inst_t inst;
    stackitem_t *x;
    stackitem_t *y;

    i = buf->tail;
    x = NULL;
    y = NULL;

    /* Add all literals from current buffer onto output, until
     * the start of the operands is reached. */
    while (i != item) {
        stack_point_new_head(prog, i);
        i = i->previous;
    }

    /* Generate instructions for operator & operand (s) */
    switch (tok) {
        case ONE:
            set_op_branch(&inst, NULL, NULL);
            stack_cat_from_item(prog, buf->head, i);
            if (stack_add_head(prog, &inst) == NULL)
                return RVM_EMEM;
        break;
        case ZERO:
            set_op_branch(&inst, NULL, NULL);
            x = stack_add_head(prog, &inst);
            stack_cat_from_item(prog, buf->head, i);
            y = stack_add_head(prog, &inst);

            if (x == NULL || y == NULL)
                return RVM_EMEM;
        break;
        case ONEZERO:
            set_op_branch(&inst, NULL, NULL);
            if (stack_add_head(prog, &inst) == NULL)
                return RVM_EMEM;

            stack_cat_from_item(prog, buf->head, i);
        break;
        case CONCAT:
            if (i != NULL) stack_cat_from_item(prog, buf->head, i);
            set_op_branch(&inst, NULL, NULL);
            x = stack_add_tail(prog, &inst);
            set_op_jmp(&inst, NULL);
            y = stack_add_head(prog, &inst);

            if (x == NULL || y == NULL)
                return RVM_EMEM;
        break;
    }

    buf->head = buf->tail = NULL;
    return 0;
}

int expand_char_range (char charc[], int *len)
{
    char rhi;
    char rlo;

    if (*lp1 > *(lp1 + 2)) {
        rhi = *lp1;
        rlo = *(lp1 + 2);
    } else {
        rhi = *(lp1 + 2);
        rlo = *lp1;
    }

    if ((rhi - rlo) > MAXCHARCLEN - (*len + 1)) {
        return RVM_ECLASSLEN;
    }

    while (rlo <= rhi) {
        charc[(*len)++] = rlo++;
    }

    return 0;
}

/* Stage 1 compiler: takes the input expression string,
 * runs it through the lexer and generates instructions for
 * the VM. The instructions returned by stage1 are incomplete;
 * branch and jmp instructions have dangling pointers. It is
 * the job of Stage 2 to assign these pointers to the correct
 * instructions. */
int stage1 (char *input, stack_t **ret)
{
    char charc[MAXCHARCLEN];
    int state;
    int pdepth;
    int charc_len;
    int tok;
    int err;

    stackitem_t *operand;
    stack_t *parens[MAXNESTPARENS];
    stack_t *buf;
    stack_t *target;
    inst_t inst;

    *ret = create_stack();
    parens[0] = create_stack();

    if (*ret == NULL || parens[0] == NULL)
        return RVM_EMEM;

    charc[0] = '\0';
    charc_len = 0;
    pdepth = 0;
    operand = NULL;
    buf = NULL;
    target = *ret;
    state = STATE_START;

    while ((tok = lex(&input)) != END) {
        if (tok < 0)
            return tok;

        switch (state) {
            case STATE_START:
                if (tok == LITERAL) {
                    set_op_char(&inst, *lp1);
                    operand = stack_add_head(parens[0], &inst);
                    buf = parens[0];
                } else if (tok == ANY) {
                    set_op_any(&inst);
                    operand = stack_add_head(parens[0], &inst);
                    buf = parens[0];
                } else if (ISOP(tok)) {
                    if (buf == NULL || buf->head == NULL)
                        return RVM_BADOP;

                    if ((err = process_op(target, operand, buf, tok)) < 0)
                        return err;

                } else if (tok == CHARC_OPEN) {
                    state = STATE_CHARC;
                } else if (tok == LPAREN) {
                    if (pdepth + 1 > MAXNESTPARENS)
                        return RVM_ENEST;

                    stack_cat(target, parens[0]);
                    parens[0] = create_stack();
                    parens[++pdepth] = create_stack();

                    if (parens[0] == NULL || parens[pdepth] == NULL)
                        return RVM_EMEM;

                    target = parens[pdepth];
                } else if (tok == RPAREN) {
                    if (pdepth < 1) {
                        return RVM_BADPAREN;
                    } else {
                        stack_cat(target, parens[0]);
                        if ((parens[0] = create_stack()) == NULL)
                            return RVM_EMEM;

                        operand = parens[pdepth]->tail;
                        buf = parens[pdepth--];
                        target = (pdepth < 1) ? *ret : parens[pdepth];
                    }
                } else if (tok == CHARC_CLOSE) {
                    return RVM_BADCLASS;
                }

            break;
            case STATE_CHARC:
                if (tok == LITERAL) {
                    charc[charc_len++] = *lp1;
                } else if (tok == CHAR_RANGE) {
                    if ((err = expand_char_range(charc, &charc_len)) != 0)
                        return err;

                } else if (tok == CHARC_CLOSE) {
                    charc[charc_len] = '\0';
                    set_op_class(&inst, charc);

                    operand = stack_add_head(parens[pdepth], &inst);
                    buf = parens[pdepth];
                    charc_len = 0;
                    charc[0] = '\0';
                    state = STATE_START;
                } else {
                    return RVM_EINVAL;
                }

            break;
        }
    }

    if (pdepth)
        return RVM_EPAREN;
    if (state == STATE_CHARC)
        return RVM_ECLASS;

    /* End of input-- anything left in the buffer
     * can be appended to the output. */
    stack_cat(*ret, parens[0]);

    /* Add the match instruction */
    set_op_match(&inst);
    stack_add_head(*ret, &inst);

    return 0;
}

int main (int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <regex>\n", argv[0]);
        exit(1);
    }

    stack_t *prog;
    int ret;

    ret = stage1(argv[1], &prog);

    printf("return code: %d\n", ret);
    if (ret < 0) exit(ret);

    printf("Input expression: %s\n", argv[1]);
    printf("after stage 1 compilation:\n");
    print_prog(prog);
    return 0;
}
