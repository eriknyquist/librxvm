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
#include "lex.h"
#include "rxvm_err.h"
#include "rxvm_common.h"
#include "stack.h"
#include "vmcode.h"

#define REP_NO_FIELD        -1
#define REP_FIELD_EMPTY     -2

#define ISOP(x) \
((x == ONE || x == ZERO || x == ONEZERO || x == ALT || x == REP) ? 1 : 0)

/* pointers for the start & end of text in
 * input string for the current token */
extern char *lp1;
extern char *lpn;

enum {STATE_START, STATE_CHARC};

static void set_op_char (inst_t *inst, char c)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_CHAR;
    inst->c = c;
}

static void set_op_any (inst_t *inst)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_ANY;
}

static void set_op_sol (inst_t *inst)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_SOL;
}

static void set_op_eol (inst_t *inst)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_EOL;
}

static void stack_reset (stack_t *stack)
{
    if (stack != NULL) {
        stack->size = 0;
        stack->head = NULL;
        stack->tail = NULL;
    }
}

static void inst_stack_cleanup (void *data)
{
    inst_t *inst;

    inst = (inst_t *) data;

    if (inst->ccs != NULL)
        free(inst->ccs);

    free(inst);
}

static void stack_stack_cleanup (void *data)
{
    stack_t *stack;

    stack = (stack_t *) data;
    stack_free(stack, inst_stack_cleanup);
}

static int process_op_rep (context_t *cp, int rep_n, int rep_m,
                           unsigned int size, stackitem_t *i)
{
    int err;

    /* {n,m} */
    if (rep_n >= 0 && rep_m >=0) {
        if ((err = code_rep_range(cp, rep_n, rep_m, size, i)) < 0) {
            return err;
        }

    /* {n,} */
    } else if (rep_n > 0 && rep_m == REP_FIELD_EMPTY) {
        if ((err = code_rep_more(cp, rep_n, size, i)) < 0) {
            return err;
        }

    /* {,m} */
    } else if (rep_n == REP_FIELD_EMPTY && rep_m > 0) {
        if ((err = code_rep_less(cp, rep_m, size, i)) < 0) {
            return err;
        }

    /* {n} */
    } else if (rep_m == REP_NO_FIELD) {
        if ((err = code_rep_n(cp, rep_n, size, i)) < 0) {
            return err;
        }

    /* Special case; treat as '*' operator */
    } else if (rep_n == 0 && rep_m == REP_FIELD_EMPTY) {
        if ((err = code_zero(cp, size, i)) < 0) {
            return err;
        }
    }

    return 0;
}

static void parse_rep (char *start, char *end, int *rep_n, int *rep_m)
{
    int *curr;

    *rep_n = REP_NO_FIELD;
    *rep_m = REP_NO_FIELD;
    curr = rep_n;

    while (++start != (end - 1)) {
        if (*start == ',') {
            curr = rep_m;
        } else {
            if (*curr == REP_NO_FIELD)
                *curr = 0;
            *curr *= 10;
            *curr += (*start - '0');
        }
    }

    if (curr == rep_m) {
        if (*rep_n == REP_NO_FIELD)
            *rep_n = REP_FIELD_EMPTY;
        if (*rep_m == REP_NO_FIELD)
            *rep_m = REP_FIELD_EMPTY;
    }
}

/* process_op: process operator 'tok' against a buffer of
 * literals 'buf', where the first operand is 'operand', and
 * append the generated instructions to 'target' */
static int process_op (context_t *cp)
{
    stackitem_t *i;
    stack_t *cur;
    unsigned int size;
    int err;
    int rep_n;
    int rep_m;

    if (cp->buf == NULL || cp->buf->head == NULL) {
        if (cp->tok == ALT) {
            cur = (stack_t *) cp->parens->head->data;
            size = cur->size;
            i = NULL;
        } else {
            return RVM_BADOP;
        }
    } else {
        size = cp->buf->size;
        i = cp->buf->tail;

        /* Add all literals from current buffer onto output, until
         * the start of the operands is reached. */
        while (i != cp->operand) {
            stack_point_new_head(cp->target, i);
            i = i->previous;
            size--;
        }
    }

    /* Generate instructions for operator & operand(s) */
    switch (cp->tok) {
        case REP:
            parse_rep(lp1, lpn, &rep_n, &rep_m);
            if ((err = process_op_rep(cp, rep_n, rep_m, size, i)) < 0) {
                return err;
            }
        break;
        case ONE:
            if ((err = code_one(cp, size, i)) < 0) {
                return err;
            }
        break;
        case ZERO:
            if ((err = code_zero(cp, size, i)) < 0) {
                return err;
            }
        break;
        case ONEZERO:
            if ((err = code_onezero(cp, size, i)) < 0) {
                return err;
            }
        break;
        case ALT:
            if ((err = code_alt(cp, size, i)) < 0) {
                return err;
            }
        break;
    }

    stack_reset(cp->buf);
    if (cp->lasttok == RPAREN) {
        free(cp->buf);
        stack_free_head(cp->parens);
    }

    cp->buf = (stack_t *) cp->parens->tail->data;

    return 0;
}

/* stage1_main_state: main logic for adding incoming literals to a stack
 * (multiple stacks, if parenthesis groups are used) so that process_op()
 * can operate on them, should any operators be seen. */
static int stage1_main_state (context_t *cp, int *state)
{
    stack_t *new;
    stack_t *cur;
    stack_t *base;
    inst_t inst;

    if (ISOP(cp->tok)) {
        return process_op(cp);
    }

    /* in the event that a parenthesis group is closed, and the following
     * token is not an operator, the buffer for the just-closed group needs
     * to be flushed into cp->target (otherwise done by process_op()) */
    if (cp->lasttok == RPAREN && cp->buf->size > 0) {
        stack_cat(cp->target, cp->buf);
        free(cp->buf);
        stack_free_head(cp->parens);
        cp->buf = (stack_t *) cp->parens->tail->data;

    }

    base = (stack_t *) cp->parens->tail->data;

    switch (cp->tok) {
        case LITERAL:
            set_op_char(&inst, *lp1);
            goto single;
        break;
        case ANY:
            set_op_any(&inst);
            goto single;
        break;
        case SOL:
            set_op_sol(&inst);
            goto single;
        break;
        case EOL:
            set_op_eol(&inst);
            goto single;
        break;
        case CHARC_OPEN:
            *state = STATE_CHARC;

            if ((cp->ccs = malloc(CHARC_BLOCK_SIZE)) == NULL)
                return RVM_EMEM;

            cp->cspace = CHARC_BLOCK_SIZE;
        break;
        case LPAREN:

            stack_cat(cp->target, base);
            stack_reset(base);


            if ((new = create_stack()) == NULL)
                    return RVM_EMEM;

            stack_add_head(cp->parens, (void *) new);
            cp->target = new;
        break;
        case RPAREN:
            if (cp->parens->head == cp->parens->tail) {
                return RVM_BADPAREN;

            } else {
                stack_cat(cp->target, base);
                attach_dangling_alt(cp);

                stack_reset(base);

                cur = (stack_t *) cp->parens->head->data;
                cp->operand = cur->tail;
                cp->buf = cur;
                cp->target = (cp->parens->head->next == cp->parens->tail) ?
                    cp->prog : (stack_t *) cp->parens->head->next->data;
            }
        break;
        case CHARC_CLOSE:
            return RVM_BADCLASS;
        break;
single:
        if ((cp->operand = stack_add_inst_head(cp->buf, &inst)) == NULL) {
            return RVM_EMEM;
        }
        break;

    }

    return 0;
}

static int charc_enlarge (context_t *cp, size_t size)
{
    char *new;

    new = realloc(cp->ccs, cp->cspace + size);

    if (new == NULL) {
        return RVM_EMEM;
    }

    cp->ccs = new;
    cp->cspace += size;
    return 0;
}

static int expand_char_range (context_t *cp)
{
    char rhi;
    char rlo;
    int err;
    size_t size;
    size_t delta;

    /* Figure out which character is highest */
    if (*lp1 > *(lp1 + 2)) {
        rhi = *lp1;
        rlo = *(lp1 + 2);
    } else {
        rhi = *(lp1 + 2);
        rlo = *lp1;
    }

    /* No. of characters in range */
    size = (size_t) rhi - rlo;

    /* Enlarge char. class buffer if needed */
    if ((cp->clen + size + 1) > cp->cspace) {
        delta = (cp->clen + size + 1) - cp->cspace;
        if ((err = charc_enlarge(cp, delta)) < 0)
            return err;
    }

    /* Expand range */
    while (rlo <= rhi) {
        cp->ccs[cp->clen++] = rlo++;
    }

    return 0;
}

/* stage1_charc_state: called repeatedly when the character class open token
 * "[" is seen, consuming tokens until the closing character "]" is seen.
 * Generates a single "class" instruction which is added to the shared
 * buffer parens[0]. */
static int stage1_charc_state (context_t *cp, int *state)
{
    int ret;

    /* Enlarge char. class buffer if it is full */
    if ((cp->clen + 1) >= cp->cspace) {
        if ((ret = charc_enlarge(cp, CHARC_BLOCK_SIZE)) < 0)
            return ret;
    } else {
        ret = 0;
    }

    switch (cp->tok) {
        case LITERAL:
            cp->ccs[cp->clen++] = *lp1;
        break;
        case CHAR_RANGE:
            ret = expand_char_range(cp);
        break;
        case CHARC_CLOSE:
            code_ccs(cp);
            *state = STATE_START;
        break;
        default:
            ret = RVM_EINVAL;
    }

    return ret;
}

static void stage1_cleanup (context_t *cp)
{
    free((stack_t *) cp->parens->tail->data);
    free(cp->parens->tail);
    free(cp->parens);
}

static void stage1_err_cleanup (context_t *cp)
{
    if (cp->ccs)
        free(cp->ccs);

    stack_free(cp->parens, stack_stack_cleanup);
    stack_free(cp->prog, inst_stack_cleanup);
}

static int stage1_init (context_t *cp, stack_t **ret)
{
    stack_t *base;

    memset(cp, 0, sizeof(context_t));

    if ((cp->parens = create_stack()) == NULL)
        return RVM_EMEM;

    if ((*ret = create_stack()) == NULL) {
        free(cp->parens);
        return RVM_EMEM;
    }

    if ((base = create_stack()) == NULL) {
        free(cp->parens);
        free(*ret);
        return RVM_EMEM;
    }

    cp->simple = NULL;
    cp->prog = *ret;
    stack_add_head(cp->parens, (void *) base);
    cp->buf = base;
    cp->target = cp->prog;
    return 0;
}

int compile_simple_backlog (context_t *cp, char *orig)
{
    inst_t inst;

    while (orig != cp->simple) {
        if (*orig == '\\') {
            ++orig;
        }

        set_op_char(&inst, *orig);
        if ((stack_add_inst_head(cp->buf, &inst)) == NULL) {
            return RVM_EMEM;
        }
        ++orig;
    }

    cp->operand = cp->buf->head;
    return 0;
}

/* Stage 1 compiler: takes the input expression string,
 * runs it through the lexer and generates an IR for stage 2. */
int stage1 (char *input, stack_t **ret)
{
    char *orig;
    int state;
    int err;

    context_t *cp;
    context_t context;

    orig = input;
    cp = &context;

    if ((err = stage1_init(cp, ret)) < 0)
        return err;

    cp->chained = 0;
    state = STATE_START;
    lex_init();

    while ((cp->tok = lex(&input)) == LITERAL) {
        cp->simple = input;
    }

    /* String contains only literal characters;
     * no compilation necessary. */
    if (cp->tok == END) {
        cp->simple = orig;
        stage1_cleanup(cp);
        stack_free(cp->prog, inst_stack_cleanup);
        *ret = NULL;
        return 0;
    } else if (cp->simple) {
        if (compile_simple_backlog(cp, orig) < 0) {
            return RVM_EMEM;
        }
        cp->simple = NULL;
    }

    do {
        if (cp->tok < 0) {
            stage1_err_cleanup(cp);
            return cp->tok;
        }

        switch (state) {
            case STATE_START:
                /* if it's not a character class, then all the
                 * metacharacters apply-- run the main state. */
                if ((err = stage1_main_state(cp, &state)) < 0) {
                    stage1_err_cleanup(cp);
                    return err;
                }

            break;
            case STATE_CHARC:
                /* inside character class-- run character class state */
                if ((err = stage1_charc_state(cp, &state)) < 0) {
                    stage1_err_cleanup(cp);
                    return err;
                }

            break;
        }
        cp->lasttok = cp->tok;
    } while ((cp->tok = lex(&input)) != END);

    if (cp->lasttok == RPAREN && cp->buf->size > 0) {
        stack_cat(cp->prog, cp->buf);
        free(cp->buf);
        stack_free_head(cp->parens);
    }

    if (state == STATE_CHARC) {
        stage1_err_cleanup(cp);
        return RVM_ECLASS;
    } else if (cp->parens->head != cp->parens->tail) {
        stage1_err_cleanup(cp);
        return RVM_EPAREN;
    }

    /* End of input-- anything left in the buffer
     * can be appended to the output. */
    stack_cat(cp->prog, (stack_t *) cp->parens->tail->data);
    attach_dangling_alt(cp);

    /* Add the match instruction */
    if ((err = code_match(cp)) < 0) {
        return err;
    }

    /* Re-use dsize member to indicate to stage2 whether
     * jmp chain optimisation is needed */
    cp->prog->dsize = cp->chained;

    stage1_cleanup(cp);
    return 0;
}
