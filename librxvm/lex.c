#include <stdio.h>
#include "lex.h"
#include "rxvm_err.h"

#define isdigit(x) ((x >= '0' && x <= '9') ? 1 : 0)

const unsigned int printable_diff = PRINTABLE_HIGH - PRINTABLE_LOW;
const unsigned int ws_diff = WS_HIGH - WS_LOW;

char newline_seen;
char *lp1;
char *lpn;

enum {STATE_START, STATE_LITERAL, STATE_RANGE, STATE_CLASS, STATE_REP,
      STATE_END};

static unsigned int literal;

static int isreserved (char x)
{
    return (x == CHARC_OPEN_SYM || x == CHARC_CLOSE_SYM) ? 1 : 0;
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

static int lex_end(int *ret, int retval, char **input, int advance)
{
    *ret = retval;
    *input += advance;
    return STATE_END;
}

static int deref(char **input)
{
    *input += 1;

    if (!**input) {
        return 1;
    }

    return 0;
}

void lex_init (void)
{
    literal = 0;
    newline_seen = 0;
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
                    if (deref(input)) {
                        return RXVM_ETRAIL;
                    }

                    lp1 += 1;
                    state = STATE_LITERAL;

                } else if (**input == CHARC_OPEN_SYM) {
                    state = trans(literal, input, CHARC_OPEN, STATE_END, &ret);
                    if (!literal) literal = 1;
                } else if (**input == CHARC_CLOSE_SYM) {
                    state = lex_end(&ret, CHARC_CLOSE, input, 1);
                    literal = 0;

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
                if (**input == '\n') {
                     newline_seen = 1;
                }

                if (**input == RANGE_SEP_SYM && literal &&
                    *(*input - 1) != DEREF_SYM) {
                    return RXVM_ERANGE;
                } else if (*(*input + 1) == RANGE_SEP_SYM && literal) {
                    state = STATE_RANGE;
                    *input += 2;
                } else {
                    state = lex_end(&ret, LITERAL, input, 1);
                }

            break;
            case STATE_RANGE:
                if (**input == DEREF_SYM) {
                    if (deref(input)) {
                        return RXVM_ETRAIL;
                    }

                    if (isprintable(**input)) {
                        state = lex_end(&ret, CHAR_RANGE, input, 1);
                    }
                } else if (isprintable(**input) && !isreserved(**input)) {
                    state = lex_end(&ret, CHAR_RANGE, input, 1);
                } else {
                    return RXVM_ERANGE;
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
                        return RXVM_ECOMMA;

                    comma = 1;
                } else if (!isdigit(**input)) {
                    return RXVM_EDIGIT;
                }

                *input += 1;
            break;
        }
    }

    if (state == STATE_REP) {
        ret = RXVM_EREP;
    }

    lpn = *input;
    return ret;
}
