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
#include "rxvm_err.h"

#define isdigit(x) ((x >= '0' && x <= '9') ? 1 : 0)

char *lp1;
char *lpn;

enum {STATE_START, STATE_LITERAL, STATE_RANGE, STATE_CLASS, STATE_DEREF,
      STATE_REP, STATE_END};

static unsigned int literal;

static int isreserved (char x)
{
    return (x == CHARC_OPEN_SYM || x == CHARC_CLOSE_SYM) ? 1 : 0;
}

static int isprintable (char x)
{
    return ((x >= PRINTABLE_LOW && x <= PRINTABLE_HIGH) ||
        (x >= WS_LOW && x <= WS_HIGH)) ? 1 : 0;
}

static int trans (int literal, char **input, int tok, int state, int *ret)
{
    int endstate;

    if (literal) {
        endstate = STATE_LITERAL;
    } else {
        endstate = state;
        if (ret) *ret = tok;
        *input += 1;
    }

    return endstate;
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
                    state = trans(literal, input, CHARC_OPEN, STATE_END, &ret);
                    if (!literal) literal = 1;
                } else if (**input == CHARC_CLOSE_SYM) {
                    state = STATE_END;
                    ret = CHARC_CLOSE;
                    literal = 0;
                    *input += 1;

                } else if (**input == REP_OPEN_SYM) {
                    state = trans(literal, input, 0, STATE_REP, NULL);
                } else if (**input == REP_CLOSE_SYM) {
                    state = trans(literal, input, RXVM_BADREP, STATE_END, &ret);
                } else if (**input == LPAREN_SYM) {
                    state = trans(literal, input, LPAREN, STATE_END, &ret);
                } else if (**input == RPAREN_SYM) {
                    state = trans(literal, input, RPAREN, STATE_END, &ret);
                } else if (**input == ONE_SYM) {
                    state = trans(literal, input, ONE, STATE_END, &ret);
                } else if (**input == ONEZERO_SYM) {
                    state = trans(literal, input, ONEZERO, STATE_END, &ret);
                } else if (**input == ZERO_SYM) {
                    state = trans(literal, input, ZERO, STATE_END, &ret);
                } else if (**input == ANY_SYM) {
                    state = trans(literal, input, ANY, STATE_END, &ret);
                } else if (**input == SOL_SYM) {
                    state = trans(literal, input, SOL, STATE_END, &ret);
                } else if (**input == EOL_SYM) {
                    state = trans(literal, input, EOL, STATE_END, &ret);
                } else if (**input == ALT_SYM) {
                    state = trans(literal, input, ALT, STATE_END, &ret);
                } else if (isprintable(**input)) {
                    state = STATE_LITERAL;
                } else {
                    return RXVM_EINVAL;
                }

            break;
            case STATE_LITERAL:
                if (**input == RANGE_SEP_SYM && literal) {
                    return RXVM_EINVAL;
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
                    return RXVM_EINVAL;
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
                    return RXVM_EINVAL;
                }

            break;
            case STATE_REP:
                if (**input == REP_CLOSE_SYM) {
                    if (!isdigit(*(*input - 1)) &&
                            !isdigit(*(lp1 + 1))) {
                        return RXVM_MREP;
                    }

                    state = STATE_END;
                    ret = REP;
                } else if (**input == ',') {
                    if (comma)
                        return RXVM_EINVAL;

                    comma = 1;
                } else if (!isdigit(**input)) {
                    return RXVM_EINVAL;
                }

                *input += 1;
            break;
        }
    }

    if (state == STATE_DEREF) {
        ret = RXVM_ETRAIL;
    } else if (state == STATE_REP) {
        ret = RXVM_EREP;
    }

    lpn = *input;
    return ret;
}
