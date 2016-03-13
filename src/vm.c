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

static int ccs_match (char *ccs, char c)
{
    while (*ccs) {
        if (*ccs == c)
            return 1;
        ++ccs;
    }

    return 0;
}

static void add_thread(uint8_t op, int *list, int *size, int val)
{
    /* if instruction is not marked as executed */
    if (op != OP_BRANCH || !cp_lookup[val]) {
        cp_lookup[val] = 1;             /* mark as executed */
        list[*size] = val;              /* add the new thread */
        ++(*size);
    }
}

int vm (regexvm_t *compiled, char *input)
{
    int *np;
    int *cp;
    int *temp;
    int nsize;
    int csize;

    int t;
    int ii;
    int ret;
    char *lastmatch;
    inst_t *ip;

    cp = malloc(sizeof(int) * compiled->size);
    np = malloc(sizeof(int) * compiled->size);
    cp_lookup = malloc(sizeof(uint8_t) * compiled->size);

    if (cp == NULL || np == NULL || cp_lookup == NULL) {
        ret = RVM_EMEM;
        goto cleanup;
    }

    nsize = 0;
    csize = 0;
    ret = 0;
    cp[csize++] = 0;
    lastmatch = NULL;
    memset(cp_lookup, 0, compiled->size);

    do {
        /* if no threads are queued for this input character,
         * then the expression cannot match, so exit */
        if (!csize)
            goto cleanup;

        /* run all the threads for this input character */
        for (t = 0; t < csize; ++t) {
            ii = cp[t];             /* index of current instruction */
            ip = compiled->exe[ii]; /* pointer to instruction data */

            switch (ip->op) {
                case OP_CHAR:
                    if (*input == ip->c) {
                        add_thread(ip->op, np, &nsize, ii + 1);
                    }

                break;
                case OP_ANY:
                    add_thread(ip->op, np, &nsize, ii + 1);
                break;
                case OP_CLASS:
                    if (ccs_match(ip->ccs, *input)) {
                        add_thread(ip->op, np, &nsize, ii + 1);
                    }

                break;
                case OP_BRANCH:
                    add_thread(ip->op, cp, &csize, ip->x);
                    add_thread(ip->op, cp, &csize, ip->y);
                break;
                case OP_JMP:
                    add_thread(ip->op, cp, &csize, ip->x);
                break;
                case OP_MATCH:
                    lastmatch = input;
                break;
            }

        }

        /* Threads saved for the next input character
         * are now threads for the current character.
         * Threads for the next character are empty again.*/
        temp = cp;
        cp = np;
        np = temp;
        csize = nsize;
        nsize = 0;
        memset(cp_lookup, 0, compiled->size);
    } while (*(input++));

    ret = (lastmatch == NULL || lastmatch < (input - 1)) ? 0 : 1;

cleanup:
    if (cp)
        free(cp);
    if (np)
        free(np);
    if (cp_lookup)
        free(cp_lookup);

    return ret;
}
