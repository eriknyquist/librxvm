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
#include "lex.h"
#include "regexvm_err.h"

#define isdigit(x) ((x >= '0' && x <= '9') ? 1 : 0)

char *lp1;
char *lpn;

enum {STATE_START, STATE_LITERAL, STATE_RANGE, STATE_CLASS, STATE_DEREF,
      STATE_REP, STATE_END};

static unsigned int literal;

static inline int isreserved (char x)
{
    return (x == CHARC_OPEN_SYM || x == CHARC_CLOSE_SYM) ? 1 : 0;
}

static inline int isprintable (char x)
{
    return ((x >= PRINTABLE_LOW && x <= PRINTABLE_HIGH) ||
        (x >= WS_LOW && x <= WS_HIGH)) ? 1 : 0;
}

static int simple_transition (int literal, char **input, int tok, int *ret)
{
    int state;

    if (literal) {
        state = STATE_LITERAL;
    } else {
        state = STATE_END;
        *ret = tok;
        *input += 1;
    }

    return state;
}

void lex_init (void)
{
    literal = 0;
}

int lex (char **input)
{
    short comma;
    int state;
    int ret;

    comma = 0;
    state = STATE_START;
    ret = INVALIDSYM;

    if (!**input) return END;

    lp1 = *input;
    while (state != STATE_END && **input) {
        switch (state) {
            case STATE_START:
                if (**input == DEREF_SYM) {
                    state = STATE_DEREF;
                    *input += 1;

                } else if (**input == CHARC_OPEN_SYM) {
                    state = STATE_END;
                    ret = CHARC_OPEN;
                    literal = 1;
                    *input += 1;

                } else if (**input == CHARC_CLOSE_SYM) {
                    state = STATE_END;
                    ret = CHARC_CLOSE;
                    literal = 0;
                    *input += 1;

                } else if (**input == REP_OPEN_SYM) {
                   state = STATE_REP;
                   *input += 1;

                } else if (**input == REP_CLOSE_SYM) {
                    return RVM_BADREP;

                } else if (**input == LPAREN_SYM) {
                    state = simple_transition(literal, input, LPAREN, &ret);
                } else if (**input == RPAREN_SYM) {
                    state = simple_transition(literal, input, RPAREN, &ret);
                } else if (**input == ONE_SYM) {
                    state = simple_transition(literal, input, ONE, &ret);
                } else if (**input == ONEZERO_SYM) {
                    state = simple_transition(literal, input, ONEZERO, &ret);
                } else if (**input == ZERO_SYM) {
                    state = simple_transition(literal, input, ZERO, &ret);
                } else if (**input == ANY_SYM) {
                    state = simple_transition(literal, input, ANY, &ret);
                } else if (**input == SOL_SYM) {
                    state = simple_transition(literal, input, SOL, &ret);
                } else if (**input == EOL_SYM) {
                    state = simple_transition(literal, input, EOL, &ret);
                } else if (**input == ALT_SYM) {
                    state = simple_transition(literal, input, ALT, &ret);
                } else if (isprintable(**input)) {
                    state = STATE_LITERAL;
                } else {
                    return RVM_EINVAL;
                }

            break;
            case STATE_LITERAL:
                if (**input == RANGE_SEP_SYM && literal) {
                    return RVM_EINVAL;
                } else if (*(*input + 1) == RANGE_SEP_SYM && literal) {
                    state = STATE_RANGE;
                    *input += 2;
                } else {
                    state = STATE_END;
                    ret = LITERAL;
                    *input += 1;
                }

            break;
            case STATE_DEREF:
                if (!**input) {
                    return RVM_EINVAL;
                } else {
                    ret = LITERAL;
                    lp1 += 1;
                }

                *input += 1;
                state = STATE_END;
            break;
            case STATE_RANGE:
                if (isprintable(**input) &&
                        !isreserved(**input)) {
                    state = STATE_END;
                    ret = CHAR_RANGE;
                    *input += 1;
                } else {
                    return RVM_EINVAL;
                }

            break;
            case STATE_REP:
                if (**input == REP_CLOSE_SYM) {
                    if (!isdigit(*(*input - 1)) &&
                            !isdigit(*(lp1 + 1))) {
                        return RVM_MREP;
                    }

                    state = STATE_END;
                    ret = REP;
                } else if (**input == ',') {
                    if (comma)
                        return RVM_EINVAL;

                    comma = 1;
                } else if (!isdigit(**input)) {
                    return RVM_EINVAL;
                }

                *input += 1;
            break;
        }
    }

    if (state == STATE_DEREF) {
        ret = RVM_ETRAIL;
    } else if (state == STATE_REP) {
        ret = RVM_EREP;
    }

    lpn = *input;
    return ret;
}
