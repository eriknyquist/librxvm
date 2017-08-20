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
#include "string_builder.h"
#include "rxvm_common.h"
#include "stack.h"
#include "vmcode.h"
#include "rxvm.h"
#include "stage1.h"
#include "setop.h"
#include "lfix.h"

#define REP_NO_FIELD        -1
#define REP_FIELD_EMPTY     -2

/* pointers for the start & end of text in
 * input string for the current token */

extern char newline_seen;
extern char *lp1;
extern char *lpn;

unsigned int alt_seen;

#define ISOP(x) \
((x == ONE || x == ZERO || x == ONEZERO || x == ALT || x == REP) ? 1 : 0)

enum {STATE_START, STATE_CHARC};

static void stack_reset (ir_stack_t *stack)
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

    if (inst != NULL) {
        if (inst->ccs != NULL)
            free(inst->ccs);

        free(inst);
    }
}

static void stack_stack_cleanup (void *data)
{
    ir_stack_t *stack;

    stack = (ir_stack_t *) data;
    stack_free(stack, inst_stack_cleanup);
}

static int process_op_rep (context_t *cp, int rep_n, int rep_m,
                           unsigned int size, stackitem_t *i)
{
    int err;

    /* {n,m} */
    if (rep_n >= 0 && rep_m >= 0) {
        if ((err = code_rep_range(cp, rep_n, rep_m, size, i)) < 0) {
            return err;
        }

    /* {n,} */
    } else if (rep_n > 0 && rep_m == REP_FIELD_EMPTY) {
        if ((err = code_rep_more(cp, rep_n, size, i)) < 0) {
            return err;
        }

    /* {,m} */
    } else if (rep_n == REP_FIELD_EMPTY && rep_m >= 0) {
        if ((err = code_rep_less(cp, rep_m, size, i)) < 0) {
            return err;
        }

    lfix_dec();

    /* {n} */
    } else if (rep_n >= 0 && rep_m == REP_NO_FIELD) {
        if ((err = code_rep_n(cp, rep_n, i)) < 0) {
            return err;
        }

    /* Special case {0,}; treat as '*' operator */
    } else if (rep_n == 0 && rep_m == REP_FIELD_EMPTY) {
        if ((err = code_zero(cp, size, i)) < 0) {
            return err;
        }

        lfix_dec();
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
    ir_stack_t *cur;
    unsigned int size;
    int err;
    int rep_n;
    int rep_m;

    if (cp->buf == NULL || cp->buf->head == NULL) {
        if (cp->tok == ALT) {
            cur = (ir_stack_t *) cp->parens->head->data;
            size = cur->size;
            i = NULL;
        } else {
            return RXVM_BADOP;
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

            lfix_dec();
        break;
        case ONEZERO:
            if ((err = code_onezero(cp, size, i)) < 0) {
                return err;
            }

            lfix_dec();
        break;
        case ALT:
            if ((err = code_alt(cp, i)) < 0) {
                return err;
            }

            if (!cp->depth) alt_seen = 1;
        break;
    }

    lfix_stop();
    stack_reset(cp->buf);
    if (cp->lasttok == RPAREN) {
        free(cp->buf);
        stack_free_head(cp->parens);
    }

    cp->buf = (ir_stack_t *) cp->parens->tail->data;
    return 0;
}

/* stage1_main_state: main logic for adding incoming literals to a stack
 * (multiple stacks, if parenthesis groups are used) so that process_op()
 * can operate on them, should any operators be seen. */
static int stage1_main_state (context_t *cp, int *state)
{
    ir_stack_t *new;
    ir_stack_t *cur;
    ir_stack_t *base;
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
        cp->buf = (ir_stack_t *) cp->parens->tail->data;

    }

    base = (ir_stack_t *) cp->parens->tail->data;

    if (cp->tok == LITERAL) {
        if (!newline_seen && !cp->depth) {
            lfix_continue(cp->orig, lp1);
        }

        set_op_char(&inst, *lp1);
        goto single;
    }

    lfix_stop();

    switch (cp->tok) {
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
            strb_init(&cp->strb, 0);
        break;
        case LPAREN:

            stack_cat(cp->target, base);
            stack_reset(base);


            if ((new = create_stack()) == NULL)
                    return RXVM_EMEM;

            stack_add_head(cp->parens, (void *) new);
            cp->target = new;
            ++cp->depth;
        break;
        case RPAREN:
            if (cp->parens->head == cp->parens->tail) {
                return RXVM_BADPAREN;

            } else {
                stack_cat(cp->target, base);
                attach_dangling_alt(cp);

                stack_reset(base);

                cur = (ir_stack_t *) cp->parens->head->data;
                cp->operand = cur->tail;
                cp->buf = cur;
                cp->target = (cp->parens->head->next == cp->parens->tail) ?
                    cp->prog : (ir_stack_t *) cp->parens->head->next->data;
            }

            --cp->depth;
        break;
        case CHARC_CLOSE:
            return RXVM_BADCLASS;
        break;
single:
        if ((cp->operand = stack_add_inst_head(cp->buf, &inst)) == NULL) {
            return RXVM_EMEM;
        }
        break;

    }

    return 0;
}

static int expand_char_range (context_t *cp)
{
    char rhi;
    char rlo;
    char tempn;
    int err;

     if (*(lp1 + 2) == DEREF_SYM) {
        tempn = *(lp1 + 3);
     } else {
        tempn = *(lp1 + 2);
     }

    /* Figure out which character is highest */
    if (*lp1 > tempn) {
        rhi = *lp1;
        rlo = tempn;
    } else {
        rhi = tempn;
        rlo = *lp1;
    }


    /* Expand range */
    while (rlo <= rhi) {
        if ((err = strb_addc(&cp->strb, rlo++)) < 0) {
            return err;
        }
    }

    return 0;
}

/* stage1_charc_state: called repeatedly when the character class open token
 * "[" is seen, consuming tokens until the closing character "]" is seen.
 * Generates a single "class" instruction which is added to the shared
 * buffer parens[0]. */
static int stage1_charc_state (context_t *cp, int *state)
{
    static uint8_t is_nchar;
    int ret = 0;

    switch (cp->tok) {
        case LITERAL:
            if (*lp1 == SOL_SYM && !is_nchar) {
                is_nchar = 1;
            } else {
                strb_addc(&cp->strb, *lp1);
            }
        break;
        case CHAR_RANGE:
            ret = expand_char_range(cp);
        break;
        case CHARC_CLOSE:
            code_ccs(cp, is_nchar);
            is_nchar = 0;
            *state = STATE_START;
        break;
        default:
            ret = RXVM_EINVAL;
    }

    return ret;
}

static void stage1_cleanup (context_t *cp)
{
    free((ir_stack_t *) cp->parens->tail->data);
    free(cp->parens->tail);
    free(cp->parens);
}

static void stage1_err_cleanup (context_t *cp)
{
    stack_free(cp->parens, stack_stack_cleanup);
    stack_free(cp->prog, inst_stack_cleanup);
    if (cp->strb.buf) free(cp->strb.buf);
}

static int stage1_init (context_t *cp, ir_stack_t **ret)
{
    ir_stack_t *base;

    memset(cp, 0, sizeof(context_t));

    if ((cp->parens = create_stack()) == NULL)
        return RXVM_EMEM;

    if ((*ret = create_stack()) == NULL) {
        free(cp->parens);
        return RXVM_EMEM;
    }

    if ((base = create_stack()) == NULL) {
        free(cp->parens);
        free(*ret);
        return RXVM_EMEM;
    }

    lfix_zero();
    alt_seen = 0;
    cp->prog = *ret;
    stack_add_head(cp->parens, (void *) base);
    cp->buf = base;
    cp->target = cp->prog;
    return 0;
}

static int compile_simple_backlog (context_t *cp)
{
    inst_t inst;
    int fixtemp;
    int delimit;
    char *orig;

    fixtemp = delimit = 0;
    orig = cp->orig;

    while (orig != cp->simple) {
        if (*orig == '\\') {
            ++delimit;
            ++orig;
        }

        if (!fixtemp && *orig == '\n') {
            fixtemp = orig - cp->orig - 1 - delimit;
        }

        set_op_char(&inst, *orig);
        if ((stack_add_inst_head(cp->buf, &inst)) == NULL) {
            return RXVM_EMEM;
        }
        ++orig;
    }

    set_ltempn((fixtemp) ? fixtemp : orig - cp->orig - 1 - delimit);
    cp->operand = cp->buf->head;
    return 0;
}

void check_simple_for_lfix (rxvm_t *compiled, char *orig)
{
    unsigned int i;
    unsigned int delimit;

    delimit = 0;
    compiled->lfix0 = 0;

    for (i = 0; orig[i]; ++i) {
        if (orig[i] == '\\') {
            ++delimit;
        } else if (orig[i] == '\n') {
            compiled->lfixn = i - 1 - delimit;
            return;
        }
    }

    compiled->lfixn = i - 1 - delimit;
}

/* Stage 1 compiler: takes the input expression string,
 * runs it through the lexer and generates an IR for stage 2. */
int stage1 (rxvm_t *compiled, char *input, ir_stack_t **ret)
{
    int state;
    int err;
    context_t ctx;

    if ((err = stage1_init(&ctx, ret)) < 0)
        return err;

    compiled->lfix0 = compiled->lfixn = 0;
    ctx.orig = input;
    state = STATE_START;
    lex_init();

    while ((ctx.tok = lex(&input)) == LITERAL) {
        ctx.simple = input;
    }

    /* String contains only literal characters;
     * no compilation necessary. */
    if (ctx.tok == END) {
        check_simple_for_lfix(compiled, ctx.orig);
        ctx.simple = ctx.orig;
        stage1_cleanup(&ctx);
        stack_free(ctx.prog, inst_stack_cleanup);
        *ret = NULL;
        return 0;
    } else if (ctx.simple) {
        if (compile_simple_backlog(&ctx) < 0) {
            return RXVM_EMEM;
        }

        ctx.simple = NULL;
    }

    do {
        if (ctx.tok < 0) {
            stage1_err_cleanup(&ctx);
            return ctx.tok;
        }

        switch (state) {
            case STATE_START:
                /* if it's not a character class, then all the
                 * metacharacters apply-- run the main state. */
                if ((err = stage1_main_state(&ctx, &state)) < 0) {
                    stage1_err_cleanup(&ctx);
                    return err;
                }

            break;
            case STATE_CHARC:
                /* inside character class-- run character class state */
                if ((err = stage1_charc_state(&ctx, &state)) < 0) {
                    stage1_err_cleanup(&ctx);
                    return err;
                }

            break;
        }

        ctx.lasttok = ctx.tok;
    } while ((ctx.tok = lex(&input)) != END);

    if (ctx.lasttok == RPAREN && ctx.buf->size > 0) {
        stack_cat(ctx.prog, ctx.buf);
        free(ctx.buf);
        stack_free_head(ctx.parens);
    }

    if (state == STATE_CHARC) {
        stage1_err_cleanup(&ctx);
        return RXVM_ECLASS;
    } else if (ctx.parens->head != ctx.parens->tail) {
        stage1_err_cleanup(&ctx);
        return RXVM_EPAREN;
    }

    /* End of input-- anything left in the buffer
     * can be appended to the output. */
    lfix_stop();
    stack_cat(ctx.prog, (ir_stack_t *) ctx.parens->tail->data);
    attach_dangling_alt(&ctx);

    /* Add the match instruction */
    if ((err = code_match(&ctx)) < 0) {
        return err;
    }

    if (alt_seen) {
        compiled->lfix0 = 0;
        compiled->lfixn = 0;
    } else {
        compiled->lfix0 = get_lfix0();
        compiled->lfixn = get_lfixn();
    }

    /* Re-use dsize member to indicate to stage2 whether
     * jmp chain optimisation is needed */
    ctx.prog->dsize = ctx.chained;
    stage1_cleanup(&ctx);
    return 0;
}
