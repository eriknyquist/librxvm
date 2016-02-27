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
#include "regexvm.h"

int ccs_match (char *ccs, char c)
{
    while (*ccs) {
        if (*ccs == c)
            return 1;
        ++ccs;
    }

    return 0;
}

int vm (regexvm_t *compiled, char *input)
{
    unsigned int *np;
    unsigned int *cp;
    unsigned int *temp;
    unsigned int nsize;
    unsigned int csize;
    unsigned int t;
    unsigned int ii;
    int ret;
    char *lastmatch;
    inst_t *ip;
    char *sp;

    if ((cp = malloc(sizeof(int) * compiled->size)) == NULL)
        return RVM_EMEM;

    if ((np = malloc(sizeof(int) * compiled->size)) == NULL)
        return RVM_EMEM;

    nsize = 0;
    csize = 0;
    cp[csize++] = 0;
    lastmatch = NULL;

    for (sp = input; *sp; ++sp) {
        /* run all the threads for this input character */
        for (t = 0; t < csize; ++t) {
            ii = cp[t];             /* index of current instruction */
            ip = compiled->exe[ii]; /* pointer to instruction data */

            switch (ip->op) {
                case OP_CHAR:
                    if (*sp == ip->c)
                        np[nsize++] = ii + 1;

                break;
                case OP_ANY:
                    np[nsize++] = ii + 1;
                break;
                case OP_CLASS:
                    if (ccs_match(ip->ccs, *sp))
                        np[nsize++] = ii + 1;
                break;
                case OP_BRANCH:
                    cp[csize++] = ip->y;
                    cp[csize++] = ip->x;
                break;
                case OP_JMP:
                    cp[csize++] = ip->x;
                break;
                case OP_MATCH:
                    lastmatch = sp;
                break;
            }

        }

        /* if no threads are queued for the next input
         * character, then the expression cannot match, so exit */
        if (!nsize) {
            free(cp);
            free(np);
            return 0;
        }

        /* Threads saved for the next input character
         * are now threads for the current character.
         * Threads for the next character are empty again.*/
        temp = cp;
        cp = np;
        np = temp;
        csize = nsize;
        nsize = 0;
    }

    for (t = 0; t < csize; t++) {
        ii = cp[t];
        ip = compiled->exe[ii];

        switch (ip->op) {
            case OP_BRANCH:
                cp[csize++] = ip->y;
                cp[csize++] = ip->x;
            break;
            case OP_JMP:
                cp[csize++] = ip->x;
            break;
            case OP_MATCH:
                lastmatch = sp;
            break;
        }
    }

    ret = (lastmatch == NULL || lastmatch < sp) ? 0 : 1;
    free(cp);
    free(np);
    return ret;
}
