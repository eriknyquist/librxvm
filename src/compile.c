#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lex.h"
#include "common.h"
#include "stack.h"

#define ISOP(x) \
((x == ONE || x == ZERO || x == ONEZERO || x == CONCAT) ? 1 : 0)

extern char *lp1;
extern char *lpn;

enum {STATE_START, STATE_CHARC};

void print_stack (stack_t *stack)
{
    stackitem_t *i = stack->head;

    while (i != NULL) {
        printf("%c", i->inst->c);
        i = i->next;
    }
}

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

void process_op (stack_t *prog, stack_t *parens[], int *pdepth, int tok)
{
    inst_t inst;
    stackitem_t *i;

    i = parens[*pdepth]->tail;

    /* Add all literals from current stack onto output, until
     * operand (which will be the head of the stack) is reached. */
    while (i != parens[*pdepth]->head) {
        stack_point_new_head(prog, i);
        i = i->previous;
    }

    /* Generate instructions for operator & operand */
    switch (tok) {
        case ONE:
            set_op_branch(&inst, NULL, NULL);
            stack_point_new_head(prog, i);
            stack_add_head(prog, &inst);
        break;
        case ZERO:
            set_op_branch(&inst, NULL, NULL);
            stack_add_head(prog, &inst);
            stack_point_new_head(prog, i);
            stack_add_head(prog, &inst);
        break;
        case ONEZERO:
            set_op_branch(&inst, NULL, NULL);
            stack_add_head(prog, &inst);
            stack_point_new_head(prog, i);
        break;
        case CONCAT:
            if (i != NULL) stack_point_new_head(prog, i);
            set_op_branch(&inst, NULL, NULL);
            stack_add_tail(prog, &inst);
            set_op_jmp(&inst, NULL);
            stack_add_head(prog, &inst);
        break;
        case CHARC_CLOSE:
            printf("CHARC_CLOSE\n");
        break;
    }

    parens[*pdepth]->head = parens[*pdepth]->tail = NULL;
}

void expand_char_range (char charc[], int *len)
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

    if ((rhi - rlo) > MAX_CHARC_LEN - (*len + 1)) {
        fprintf(stderr, "Character class too large");
        exit(1);
    }

    while (rlo <= rhi) {
        charc[(*len)++] = rlo++;
    }
}

stack_t *stage1 (char *input)
{
    char charc[MAX_CHARC_LEN];
    int state;
    int pdepth;
    int charc_len;
    int tok;
    stack_t *parens[MAX_NEST_PARENS];
    stack_t *ret;
    inst_t inst;

    ret = create_stack();
    parens[0] = create_stack();

    charc[0] = '\0';
    charc_len = 0;
    pdepth = 0;
    state = STATE_START;

    while ((tok = lex(&input)) != END) {
        switch (state) {
            case STATE_START:
                if (tok == LITERAL) {
                    set_op_char(&inst, *lp1);
                    stack_add_head(parens[pdepth], &inst);
                } else if (tok == ANY) {
                    set_op_any(&inst);
                    stack_add_head(parens[pdepth], &inst);
                } else if (ISOP(tok)) {
                    process_op(ret, parens, &pdepth, tok);
                } else if (tok == CHARC_OPEN) {
                    state = STATE_CHARC;
                }

            break;
                //parens[++pdepth] = create_stack();
            case STATE_CHARC:
                if (tok == LITERAL) {
                    charc[charc_len++] = *lp1;
                } else if (tok == CHAR_RANGE) {
                    expand_char_range(charc, &charc_len);
                } else if (tok == CHARC_CLOSE) {
                    charc[charc_len] = '\0';
                    set_op_class(&inst, charc);
                    stack_add_head(parens[pdepth], &inst);
                    charc_len = 0;
                    charc[0] = '\0';
                    state = STATE_START;
                } else {
                    fprintf(stderr, "Invalid symbol in character class\n");
                    exit(1);
                }
            break;
        }
    }

    /* End of input-- anything left in the buffer
     * can be appended to the output. */
    stack_cat(&ret, &parens[pdepth]);

    /* Add the match instruction */
    set_op_match(&inst);
    stack_add_head(ret, &inst);

    return ret;
}

int main (void)
{
    stack_t *prog;
    char *s = "aa+b*\\*aa|[A-Fa-f.~]*";

    prog = stage1(s);

    printf("Input expression: %s\n", s);
    printf("after stage 1 compilation:\n");
    print_prog(prog);
    return 0;
}
