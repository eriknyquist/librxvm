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

#include <string.h>
#include "rxvm_common.h"

void set_op_class (inst_t *inst, char *ccs)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_CLASS;
    inst->ccs = ccs;
}

void set_op_nclass (inst_t *inst, char *ccs)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_NCLASS;
    inst->ccs = ccs;
}

void set_op_branch (inst_t *inst, int x, int y)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_BRANCH;
    inst->x = x;
    inst->y = y;
}

void set_op_jmp (inst_t *inst, int x)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_JMP;
    inst->x = x;
}

void set_op_match (inst_t *inst)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_MATCH;
}

void set_op_char (inst_t *inst, char c)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_CHAR;
    inst->c = c;
}

void set_op_any (inst_t *inst)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_ANY;
}

void set_op_sol (inst_t *inst)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_SOL;
}

void set_op_eol (inst_t *inst)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_EOL;
}
