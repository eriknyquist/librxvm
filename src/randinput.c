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
#include "string_builder.h"
#include "lex.h"

#define DEFAULT_WS_PROB        10
#define DEFAULT_GEN_PROB       50

static unsigned int rand_range (unsigned int low, unsigned int high)
{
        return (unsigned int) low + (rand() % ((high - low) + 1));
}

static int choice (int prob)
{
    return (rand_range(0, 100) < prob);
}

char *rxvm_gen (rxvm_t *compiled, rxvm_gencfg_t *cfg)
{
    inst_t **exe;
    inst_t *inst;
    char *ret;
    int val;
    char rand;
    size_t size;
    unsigned int ip;
    unsigned int ix;
    strb_t strb;

    if (compiled->simple) {
        size = sizeof(char) * (strlen(compiled->simple) + 1);
        if ((ret = malloc(size)) == NULL) {
            return NULL;
        }

        memcpy(ret, compiled->simple, size);
        return ret;
    }

    strb_init(&strb, 50);
    ip = 0;

    exe = compiled->exe;
    inst = exe[ip];
    while (inst->op != OP_MATCH) {
        switch (inst->op) {
            case OP_CHAR:
                strb_addc(&strb, inst->c);
                ++ip;
            break;
            case OP_ANY:
                val = (cfg) ? cfg->whitespace : DEFAULT_WS_PROB;
                if (choice(val)) {
                    rand = (char) rand_range(WS_LOW, WS_HIGH);
                    strb_addc(&strb, rand);
                    ++ip;
                } else {
                    rand = (char) rand_range(PRINTABLE_LOW, PRINTABLE_HIGH);
                    strb_addc(&strb, rand);
                    ++ip;
                }

            break;
            case OP_SOL:
                if (strb.size &&
                        strb.buf[strb.size - 1] != '\n') {
                    strb_addc(&strb, '\n');
                }
                ++ip;
            break;
            case OP_EOL:
                if (compiled->size > ip && (exe[ip + 1] != OP_CHAR
                            || exe[ip + 1]->c != '\n')) {
                    strb_addc(&strb, '\n');
                }
                ++ip;
            break;
            case OP_CLASS:
                ix = rand_range(0, strlen(inst->ccs) - 1);
                strb_addc(&strb, inst->ccs[ix]);
                ++ip;
            break;
            case OP_BRANCH:
                val = (cfg) ? cfg->generosity : DEFAULT_GEN_PROB;
                ip = (choice(val)) ? inst->x : inst->y;
            break;
            case OP_JMP:
                ip = inst->x;
            break;
        }

        inst = exe[ip];
    }

    strb_addc(&strb, '\0');
    return strb.buf;
}
