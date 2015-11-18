#include <stdio.h>
#include <stdlib.h>
#include "lex.h"

#define PRINTABLE_LOW      ' '  /* ASCII 0x20 */
#define PRINTABLE_HIGH     '~'  /* ASCII 0x7E */

static char *old;
static char *new;
static unsigned int literal;

static inline int isreserved (char x)
{
    return (x == CHARC_OPEN_SYM || x == CHARC_CLOSE_SYM) ? 1 : 0;
}

static inline int isprintable (char x)
{
    return (x >= PRINTABLE_LOW && x <= PRINTABLE_HIGH) ? 1 : 0;
}

char *get_token_text (void)
{
    char *ret;
    size_t size;
    unsigned int i;

    size = new - old;
    ret = malloc(size + 1);
    for (i = 0; i < size; i++)
        ret[i] = old[i];

    ret[size] = '\0';
    return ret;
}

int lex (char **input)
{
    int state = STATE_START;
    int ret = INVALIDSYM;

    if (!**input) return END;

    old = *input;
    while (state != STATE_END && **input) {
        switch (state) {
            case STATE_START:
                if (**input == DEREF_SYM) {
                    state = STATE_DEREF;
                    (*input)++;
                } else if (**input == CHARC_OPEN_SYM) {
                    state = STATE_END;
                    ret = CHARC_OPEN;
                    literal = 1;

                    (*input)++;
                } else if (**input == CHARC_CLOSE_SYM) {
                    state = STATE_END;
                    ret = CHARC_CLOSE;

                    literal = 0;
                    (*input)++;
                } else if (**input == ONE_SYM) {
                    if (literal) {
                        state = STATE_LITERAL;
                    } else {
                        state = STATE_END;
                        ret = ONE;
                        (*input)++;
                    }

                } else if (**input == ZERO_SYM) {
                    if (literal) {
                        state = STATE_LITERAL;
                    } else {
                        state = STATE_END;
                        ret = ZERO;
                        (*input)++;
                    }

                } else if (**input == ANY_SYM) {
                    if (literal) {
                        state = STATE_LITERAL;
                    } else {
                        state = STATE_END;
                        ret = ANY;
                        (*input)++;
                    }

                } else if (isprintable(**input)) {
                    state = STATE_LITERAL;
                } else {
                    state = STATE_END;
                    ret = INVALIDSYM;
                    (*input)++;
                }

            break;

            case STATE_LITERAL:
                if (**input == RANGE_SEP_SYM && literal) {
                    state = STATE_END;
                    ret = INVALIDSYM;
                    (*input)++;
                } else if (*(*input + 1) == RANGE_SEP_SYM && literal) {
                    state = STATE_RANGE;
                    *input += 2;
                } else {
                    state = STATE_END;
                    ret = LITERAL;
                    (*input)++;
                }

            break;

            case STATE_DEREF:
                if (!**input) {
                    ret = INVALIDSYM;
                } else {
                    ret = LITERAL;
                    old++;
                }

                (*input)++;
                state = STATE_END;
            break;

            case STATE_RANGE:
                if (isprintable(**input) && !isreserved(**input)) {
                    state = STATE_END;
                    ret = CHAR_RANGE;
                    (*input)++;
                } else {
                    state = STATE_END;
                    ret = INVALIDSYM;
                }

            break;
        }
    }

    new = *input;
    return ret;
}
