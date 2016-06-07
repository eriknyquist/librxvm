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

#define tolower(x) ((x <= 'Z' && x >= 'A') ? x + 32 : x)

#if (DBGF)
int fcnt;

void regexvm_print_pointer (FILE *fp, regexvm_t *compiled, int point)
{
    unsigned int i;
    inst_t *inst;

    for (i = 0; i < compiled->size; i++) {
        inst = compiled->exe[i];

        if (i == point) {
            fprintf(fp, "----> ");
        } else {
            fprintf(fp, "      ");
        }

        switch(inst->op) {
            case OP_CHAR:
                fprintf(fp, "%-4dchar %c\n", i, inst->c);
            break;
            case OP_ANY:
                fprintf(fp, "%-4dany\n", i);
            break;
            case OP_SOL:
                fprintf(fp, "%-4dsol\n", i);
            break;
            case OP_EOL:
                fprintf(fp, "%-4deol\n", i);
            break;
            case OP_CLASS:
                fprintf(fp, "%-4dclass %s\n", i, inst->ccs);
            break;
            case OP_BRANCH:
                fprintf(fp, "%-4dbranch %d %d\n", i, inst->x, inst->y);
            break;
            case OP_JMP:
                fprintf(fp, "%-4djmp %d\n", i, inst->x);
            break;
            case OP_MATCH:
                fprintf(fp, "%-4dmatch\n", i);
            break;
        }
    }
}

void print_threads_state (regexvm_t *compiled, threads_t *tm, int cur,
                          char *input)
{
    FILE *fp;
    char *sot;
    char filename[50];
    int i;

    sot = input - tm->chars;
    snprintf(filename, 50, ".rvm_dbgf/%d.dbgf", ++fcnt);

    if ((fp = fopen(filename, "w")) == NULL) {
        printf("Error opening file %s for writing\n", filename);
        return;
    }

    fprintf(fp, "%s\n", sot);
    for (i = 0; i < (tm->chars - 1); ++i) {
        fprintf(fp, " ");
    }
    fprintf(fp, "^\n\n");
    regexvm_print_pointer(fp, compiled, tm->cp[cur]);
    fprintf(fp, "\n");

    fprintf(fp, "Threads for current input character:\n");
    fprintf(fp, "-----------------------------------\n\n");
    for (i = 0; i < tm->csize; ++i) {
        fprintf(fp, "%-4d", tm->cp[i]);
    }
    fprintf(fp, "\n");
    for (i = 0; i < cur; ++i) {
        fprintf(fp, "    ");
    }
    fprintf(fp, "^\n\n\n");

    fprintf(fp, "Threads for next input character:\n");
    fprintf(fp, "--------------------------------\n\n");
    for (i = 0; i < tm->nsize; ++i) {
        fprintf(fp, "%-4d", tm->np[i]);
    }
    fprintf(fp, "\n");
    fclose(fp);
}
#endif  /* DBGF */

int char_match (uint8_t icase, char a, char b)
{
    return (icase) ? tolower(a) == tolower(b) : a == b;
}

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

static void vm_zero (threads_t *tm, unsigned int size)
{
    memset(tm->cp_lookup, 0, size);
    memset(tm->np_lookup, 0, size);
    memset(tm->cp, 0, sizeof(int) * size);
    memset(tm->np, 0, sizeof(int) * size);
}

int vm_execute (threads_t *tm, regexvm_t *compiled)
{
    char C;
    int t;
    int ii;
    int *dtemp;
    uint8_t *ltemp;
    inst_t *ip;

    tm->nsize = 0;
    tm->csize = 0;

    tm->match_start = tm->chars;
    vm_zero(tm, compiled->size);
    add_thread_curr(tm, 0);

#if (DBGF)
    fcnt = 0;
#endif

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
            ip = compiled->exe[ii]; /* pointer to instruction data */

#if (DBGF)
            print_threads_state(compiled, tm, t, *((char **)tm->getchar_data));
#endif

            switch (ip->op) {
                case OP_CHAR:
                    if (char_match(tm->icase, C, ip->c)) {
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
                    if (ccs_match(tm->icase, ip->ccs, C)) {
                        add_thread_next(tm, ii + 1);
                    }

                break;
                case OP_BRANCH:
                    add_thread_curr(tm, ip->x);
                    add_thread_curr(tm, ip->y);
                break;
                case OP_JMP:
                    add_thread_curr(tm, ip->x);
                break;
                case OP_MATCH:
                    tm->match_end = tm->chars;
                    if (tm->nongreedy) return 1;
                break;
            }
#if (DBGF)
            print_threads_state(compiled, tm, t, *((char**)tm->getchar_data));
#endif
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

void vm_cleanup(threads_t *tm)
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
