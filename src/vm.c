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
#include "regexvm.h"
#include "vm.h"

static int ccs_match (char *ccs, char c)
{
    while (*ccs) {
        if (*ccs == c)
            return 1;
        ccs += 1;
    }

    return 0;
}

static void add_thread (int *list, uint8_t *lookup, int *lsize, int val)
{
    if (!lookup[val]) {
        lookup[val] = 1;
        list[(*lsize)++] = val;
    }
}

int vm_execute (threads_t *tm, regexvm_t *compiled, char **input)
{
    int t;
    int ii;
    int *dtemp;
    uint8_t *ltemp;
    inst_t *ip;

    tm->nsize = 0;
    tm->csize = 0;
    tm->match_end = NULL;

    memset(tm->cp_lookup, 0, compiled->size);
    memset(tm->np_lookup, 0, compiled->size);
    memset(tm->cp, 0, compiled->size);
    memset(tm->np, 0, compiled->size);

    add_thread(tm->cp, tm->cp_lookup, &tm->csize, 0);
    tm->match_start = *input;

    do {
        /* if no threads are queued for this input character,
         * then the expression cannot match, so exit */
        if (**input && !tm->csize) {
            return 1;
        }

        /* run all the threads for this input character */
        for (t = 0; t < tm->csize; ++t) {
            ii = tm->cp[t];    /* index of current instruction */
            ip = compiled->exe[ii]; /* pointer to instruction data */

            switch (ip->op) {
                case OP_CHAR:
                    if (**input == ip->c) {
                        add_thread(tm->np, tm->np_lookup, &tm->nsize, ii + 1);
                    }

                break;
                case OP_ANY:
                    add_thread(tm->np, tm->np_lookup, &tm->nsize, ii + 1);
                break;
                case OP_CLASS:
                    if (ccs_match(ip->ccs, **input)) {
                        add_thread(tm->np, tm->np_lookup, &tm->nsize, ii + 1);
                    }

                break;
                case OP_BRANCH:
                    add_thread(tm->cp, tm->cp_lookup, &tm->csize, ip->x);
                    add_thread(tm->cp, tm->cp_lookup, &tm->csize, ip->y);
                break;
                case OP_JMP:
                    add_thread(tm->cp, tm->cp_lookup, &tm->csize, ip->x);
                break;
                case OP_MATCH:
                    tm->match_end = *input;
                break;
            }
        }

        /* Threads saved for the next input character
         * are now threads for the current character.
         * Threads for the next character are empty again.*/
        dtemp = tm->cp;
        tm->cp = tm->np;
        tm->np = dtemp;
        tm->csize = tm->nsize;
        tm->nsize = 0;

        /* swap lookup tables */
        ltemp = tm->cp_lookup;
        tm->cp_lookup = tm->np_lookup;
        tm->np_lookup = ltemp;

        memset(tm->np_lookup, 0, compiled->size);

    } while (*(*input)++);
    return 0;
}

int vm_init (threads_t *tm, unsigned int size)
{
    tm->cp_lookup = tm->np_lookup = NULL;
    tm->cp = tm->np = NULL;

    if ((tm->cp = malloc(size * sizeof(int))) == NULL)
        return RVM_EMEM;

    if ((tm->np = malloc(size * sizeof(int))) == NULL)
        return RVM_EMEM;

    if ((tm->cp_lookup = malloc(size)) == NULL)
        return RVM_EMEM;

    if ((tm->np_lookup = malloc(size)) == NULL)
        return RVM_EMEM;

    return 0;
}

void vm_free(threads_t *tm)
{
    if (tm->cp)
        free(tm->cp);
    if (tm->np)
        free(tm->np);
    if (tm->cp_lookup)
        free(tm->cp_lookup);
    if (tm->np_lookup)
        free(tm->np_lookup);
}
