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

uint8_t *cp_lookup;
uint8_t *np_lookup;

static int ccs_match (char *ccs, char c)
{
    while (*ccs) {
        if (*ccs == c)
            return 1;
        ++ccs;
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

int vm (regexvm_t *compiled, char *input)
{
    int *np;
    int *cp;
    int *dtemp;
    uint8_t *ltemp;
    int nsize;
    int csize;

    int t;
    int ii;
    int ret;
    char *lastmatch;
    inst_t *ip;

    cp = malloc(compiled->size * sizeof(int));
    np = malloc(compiled->size * sizeof(int));
    cp_lookup = malloc(compiled->size);
    np_lookup = malloc(compiled->size);

    if (cp == NULL || np == NULL || cp_lookup == NULL || np_lookup == NULL) {
        ret = RVM_EMEM;
        goto cleanup;
    }

    nsize = 0;
    csize = 0;
    ret = 0;
    lastmatch = NULL;
    memset(cp_lookup, 0, compiled->size);
    memset(np_lookup, 0, compiled->size);

    add_thread(cp, cp_lookup, &csize, 0);
    do {
        /* if no threads are queued for this input character,
         * then the expression cannot match, so exit */
        if (input && !csize) {
            goto cleanup;
        }

        /* run all the threads for this input character */
        for (t = 0; t < csize; ++t) {
            ii = cp[t];             /* index of current instruction */
            ip = compiled->exe[ii]; /* pointer to instruction data */

            switch (ip->op) {
                case OP_CHAR:
                    if (*input == ip->c) {
                        add_thread(np, np_lookup, &nsize, ii + 1);
                    }

                break;
                case OP_ANY:
                    add_thread(np, np_lookup, &nsize, ii + 1);
                break;
                case OP_CLASS:
                    if (ccs_match(ip->ccs, *input)) {
                        add_thread(np, np_lookup, &nsize, ii + 1);
                    }

                break;
                case OP_BRANCH:
                    add_thread(cp, cp_lookup, &csize, ip->x);
                    add_thread(cp, cp_lookup, &csize, ip->y);
                break;
                case OP_JMP:
                    add_thread(cp, cp_lookup, &csize, ip->x);
                break;
                case OP_MATCH:
                    lastmatch = input;
                break;
            }
        }

        /* Threads saved for the next input character
         * are now threads for the current character.
         * Threads for the next character are empty again.*/
        dtemp = cp;
        cp = np;
        np = dtemp;
        csize = nsize;
        nsize = 0;

        /* swap lookup tables */
        ltemp = cp_lookup;
        cp_lookup = np_lookup;
        np_lookup = ltemp;

        memset(np_lookup, 0, compiled->size);

    } while (*(input++));

    ret = (lastmatch < (input - 1) || lastmatch == NULL) ? 0 : 1;

cleanup:
    if (cp)
        free(cp);
    if (np)
        free(np);
    if (cp_lookup)
        free(cp_lookup);
    if (np_lookup)
        free(np_lookup);

    return ret;
}
