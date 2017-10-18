#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxvm.h"
#include "vm.h"

#define VM_TABLE_SIZE(size) ((size * 2) + (sizeof(int) * size * 2))

const int udiff = 'Z' - 'A';

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

int vm_execute (threads_t *tm, rxvm_t *compiled, int64_t input_limit)
{
    inst_t inst;
    int *dtemp;
    uint8_t *ltemp;
    char C;
    int t;
    int i;
    int ii;

    C = 0;
    tm->nsize = 0;
    tm->csize = 0;

    memset(tm->table_base, 0, VM_TABLE_SIZE(compiled->size));
vm_start:
    tm->match_start = tm->chars;
    add_thread_curr(tm, 0);

    while (1) {
        tm->lastinput = C;
        C = (*tm->getchar)(tm->getchar_data);
        ++tm->chars;

skip_readchar:
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
                    if (C != '\n') {
                        add_thread_next(tm, ii + 1);
                    }

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
                case OP_NCLASS:
                    if (!ccs_match(tm->icase, inst.ccs, C)) {
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

        if (C == tm->endchar)
            break;

        if (!tm->nsize) {
            /* If we've used no thread slots for the next character, avoid
             * the full swap and do this shorter version. Also, if tm->search
             * is set, we can avoid returning from vm_execute() and just jump
             * back to the start instead */
            if (!tm->search || tm->match_end) {
                return 1;
            }

            for (i = 0; i < tm->csize; ++i) {
                tm->cp_lookup[tm->cp[i]] = 0;
            }

            tm->csize = 0;

            if ((tm->chars - 1) != tm->match_start) {
                tm->match_start = tm->chars - 1;
                add_thread_curr(tm, 0);
                goto skip_readchar;
            }

            goto vm_start;
        } else {
            /* Full swap. Threads saved for the next input character
             * are now threads for the current character.
             * Threads for the next character are empty again.*/
            dtemp = tm->cp;
            tm->cp = tm->np;
            tm->np = dtemp;
            tm->csize = tm->nsize;
            tm->nsize = 0;

            /* Swap lookup tables */
            ltemp = tm->cp_lookup;
            tm->cp_lookup = tm->np_lookup;
            tm->np_lookup = ltemp;
            memset(tm->np_lookup, 0, compiled->size);
        }
    }

    return 0;
}

int vm_init (threads_t *tm, unsigned int size)
{
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
    free(tm->table_base);
}
