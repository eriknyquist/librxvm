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
            printf("%d\tbranch %d %d", num, num + inst->x, num + inst->y);
        break;

        case OP_JMP:
            printf("%d\tjmp %d", num, num + inst->x);
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
}

void set_op_class (inst_t *inst, char *ccs)
{
    inst->op = OP_CLASS;
    inst->ccs = ccs;
}

void set_op_any (inst_t *inst)
{
    inst->op = OP_ANY;
}

void set_op_branch (inst_t *inst, int x, int y)
{
    inst->op = OP_BRANCH;
    inst->x = x;
    inst->y = y;
}

void set_op_jmp (inst_t *inst, int x)
{
    inst->op = OP_JMP;
    inst->x = x;
}

void set_op_match (inst_t *inst)
{
    inst->op = OP_MATCH;
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
static int process_op (context_t *cp)
{
    stackitem_t *i;
    stackitem_t *x;
    stackitem_t *y;
    unsigned int size;
    unsigned int savedsize;
    static inst_t inst;

    savedsize = size = cp->buf->size;
    i = cp->buf->tail;
    x = NULL;
    y = NULL;

    /* Add all literals from current buffer onto output, until
     * the start of the operands is reached. */
    while (i != cp->operand) {
        stack_point_new_head(cp->target, i);
        i = i->previous;
        size--;
    }

    /* Generate instructions for operator & operand (s) */
    switch (cp->tok) {
        case ONE:
            set_op_branch(&inst, -(cp->buf->size), 1);
            stack_cat_from_item(cp->target, cp->buf->head, i);
            if (stack_add_head(cp->target, &inst) == NULL)
                return RVM_EMEM;
        break;
        case ZERO:
            set_op_branch(&inst, size + 2, 1);
            x = stack_add_head(cp->target, &inst);
            stack_cat_from_item(cp->target, cp->buf->head, i);
            set_op_branch(&inst, 1, -(size));
            y = stack_add_head(cp->target, &inst);

            if (x == NULL || y == NULL)
                return RVM_EMEM;
        break;
        case ONEZERO:
            set_op_branch(&inst, 1, size + 1);
            if (stack_add_head(cp->target, &inst) == NULL)
                return RVM_EMEM;

            stack_cat_from_item(cp->target, cp->buf->head, i);
        break;
        case CONCAT:
            if (i != NULL) stack_cat_from_item(cp->target, cp->buf->head, i);
            set_op_branch(&inst, 1, savedsize + 2);
            x = stack_add_tail(cp->target, &inst);
            set_op_jmp(&inst, 0);
            cp->dangling_cat = stack_add_head(cp->target, &inst);

            if (x == NULL || cp->dangling_cat == NULL)
                return RVM_EMEM;

        break;
    }

    cp->buf->head = cp->buf->tail = NULL;
    cp->buf->size = 0;
    return 0;
}

static int expand_char_range (char charc[], unsigned int *len)
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

static inline void attach_cat(context_t *ctx, unsigned int size)
{
    if (ctx->dangling_cat != NULL) {
        ctx->dangling_cat->inst->x = size + 1;
        ctx->dangling_cat = NULL;
    }
}

static int stage1_main_state(context_t *cp, stack_t *parens[], int *state)
{
    int err;
    static inst_t inst;

    if (ISOP(cp->tok)) {
        if (cp->buf == NULL || cp->buf->head == NULL)
            return RVM_BADOP;

        if ((err = process_op(cp)) < 0)
            return err;

    } else {
        if (cp->lasttok == RPAREN && cp->buf->size > 0) {
            stack_cat(cp->prog, cp->buf);
        }

        if (cp->tok == LITERAL) {
            set_op_char(&inst, *lp1);
            cp->operand = stack_add_head(parens[0], &inst);
            cp->buf = parens[0];

        } else if (cp->tok == ANY) {
            set_op_any(&inst);
            cp->operand = stack_add_head(parens[0], &inst);
            cp->buf = parens[0];

        } else if (cp->tok == CHARC_OPEN) {
            *state = STATE_CHARC;

        } else if (cp->tok == LPAREN) {
            if (cp->pdepth + 1 > MAXNESTPARENS)
                return RVM_ENEST;

            stack_cat(cp->target, parens[0]);
            if ((parens[0] = create_stack()) == NULL)
                return RVM_EMEM;

            if ((parens[++cp->pdepth] = create_stack()) == NULL)
                return RVM_EMEM;

            if (parens[0] == NULL || parens[cp->pdepth] == NULL)
                return RVM_EMEM;

            cp->target = parens[cp->pdepth];

        } else if (cp->tok == RPAREN) {
            if (cp->pdepth < 1) {
                return RVM_BADPAREN;

            } else {
                stack_cat(cp->target, parens[0]);
                attach_cat(cp, cp->target->size);
                if ((parens[0] = create_stack()) == NULL)
                    return RVM_EMEM;

                cp->operand = parens[cp->pdepth]->tail;
                cp->buf = parens[cp->pdepth--];
                cp->target = (cp->pdepth < 1) ? cp->prog : parens[cp->pdepth];
            }

        } else if (cp->tok == CHARC_CLOSE) {
            return RVM_BADCLASS;
        }
    }

    return 0;
}

static int stage1_charc_state(context_t *cp, stack_t *parens[], char charc[],
                       int *state)
{
    int err;
    static inst_t inst;

    if (cp->tok == LITERAL) {
        charc[cp->clen++] = *lp1;
    } else if (cp->tok == CHAR_RANGE) {
        if ((err = expand_char_range(charc, &cp->clen)) != 0)
            return err;

    } else if (cp->tok == CHARC_CLOSE) {
        charc[cp->clen] = '\0';
        set_op_class(&inst, charc);

        cp->operand = stack_add_head(parens[cp->pdepth], &inst);
        cp->buf = parens[cp->pdepth];
        cp->clen = 0;
        charc[0] = '\0';
        *state = STATE_START;
    } else {
        return RVM_EINVAL;
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
    static char charc[MAXCHARCLEN];
    static int state;
    static int err;

    static context_t context, *cp;
    static stack_t *parens[MAXNESTPARENS];
    static inst_t inst;

    cp = &context;
    *ret = create_stack();
    cp->prog = *ret;
    parens[0] = create_stack();

    if (*ret == NULL || parens[0] == NULL)
        return RVM_EMEM;

    charc[0] = '\0';
    cp->pdepth = 0;
    cp->target = cp->prog;
    cp->dangling_cat = NULL;
    state = STATE_START;

    while ((cp->tok = lex(&input)) != END) {
        if (cp->tok < 0)
            return cp->tok;

        switch (state) {
            case STATE_START:
                if ((err = stage1_main_state(cp, parens, &state)) < 0)
                    return err;

            break;
            case STATE_CHARC:
                if ((err = stage1_charc_state(cp, parens, charc, &state)) < 0)
                    return err;

            break;
        }
        cp->lasttok = cp->tok;
    }

    if (cp->lasttok == RPAREN && cp->buf->size > 0) {
        stack_cat(cp->prog, cp->buf);
    }

    if (cp->pdepth)
        return RVM_EPAREN;
    if (state == STATE_CHARC)
        return RVM_ECLASS;

    attach_cat(cp, parens[0]->size);

    /* End of input-- anything left in the buffer
     * can be appended to the output. */
    stack_cat(cp->prog, parens[0]);

    /* Add the match instruction */
    set_op_match(&inst);
    stack_add_head(cp->prog, &inst);

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
    printf("after stage 1 compilation:\n\n");
    print_prog(prog);
    printf("size: %u\n", prog->size);
    return 0;
}
