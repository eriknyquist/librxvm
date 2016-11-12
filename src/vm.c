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
#include "rxvm.h"
#include "vm.h"

#define VM_TABLE_SIZE(size) ((size * 2) + (sizeof(int) * size * 2))

static int ccs_match (uint8_t icase, char *ccs, char c)
{
    while (*ccs) {
        if (char_match(icase, *ccs, c))
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

static int is_sol (threads_t *tm)
{
    if (tm->chars == 1) {
        return 1;
    } else {
        return (tm->multiline && tm->lastinput == '\n') ? 1 : 0;
    }
}

static int is_eol (threads_t *tm, char c)
{
    if (c == tm->endchar) {
        return 1;
    } else {
        return (tm->multiline && c == '\n') ? 1 : 0;
    }
}

static void add_thread_curr (threads_t *tm, int val)
{
    add_thread(tm->cp, tm->cp_lookup, &tm->csize, val);
}

static void add_thread_next (threads_t *tm, int val)
{
    add_thread(tm->np, tm->np_lookup, &tm->nsize, val);
}

int vm_execute (threads_t *tm, rxvm_t *compiled)
{
    char C;
    int t;
    int ii;
    int *dtemp;
    uint8_t *ltemp;
    inst_t inst;

    tm->nsize = 0;
    tm->csize = 0;

    memset(tm->table_base, 0, VM_TABLE_SIZE(compiled->size));
    tm->match_start = tm->chars;
    add_thread_curr(tm, 0);

    do {
        /* if no threads are queued for this input character,
         * then the expression cannot match, so exit */
        if (!tm->csize) {
            return 1;
        }

        C = (*tm->getchar)(tm->getchar_data);
        ++tm->chars;

        /* run all the threads for this input character */
        for (t = 0; t < tm->csize; ++t) {
            ii = tm->cp[t];    /* index of current instruction */
            inst = compiled->exe[ii]; /* pointer to instruction data */

            switch (inst.op) {
                case OP_CHAR:
                    if (char_match(tm->icase, C, inst.c)) {
                        add_thread_next(tm, ii + 1);
                    }

                break;
                case OP_ANY:
                    add_thread_next(tm, ii + 1);
                break;
                case OP_SOL:
                    if (is_sol(tm)) {
                        add_thread_curr(tm, ii + 1);
                    }

                break;
                case OP_EOL:
                    if (is_eol(tm, C)) {
                        add_thread_curr(tm, ii + 1);
                    }

                break;
                case OP_CLASS:
                    if (ccs_match(tm->icase, inst.ccs, C)) {
                        add_thread_next(tm, ii + 1);
                    }

                break;
                case OP_BRANCH:
                    add_thread_curr(tm, inst.x);
                    add_thread_curr(tm, inst.y);
                break;
                case OP_JMP:
                    add_thread_curr(tm, inst.x);
                break;
                case OP_MATCH:
                    tm->match_end = tm->chars;
                    if (tm->nongreedy) return 1;
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
        tm->lastinput = C;
    } while (C != tm->endchar);
    return 0;
}

int vm_init (threads_t *tm, unsigned int size)
{
    tm->cp_lookup = tm->np_lookup = NULL;
    tm->cp = tm->np = NULL;
    tm->match_start = tm->match_end = 0;
    tm->lastinput = 0;
    tm->chars = 0;

    if ((tm->table_base = (uint8_t *)malloc(VM_TABLE_SIZE(size))) == NULL)
        return RXVM_EMEM;

    tm->cp = (int *)tm->table_base;
    tm->np = (int *)(tm->table_base + (sizeof(int) * size));
    tm->cp_lookup = tm->table_base + (sizeof(int) * size * 2);
    tm->np_lookup = tm->table_base + (sizeof(int) * size * 2) + size;

    return 0;
}

void vm_cleanup(threads_t *tm)
{
    if (tm->table_base)
        free(tm->table_base);
}
