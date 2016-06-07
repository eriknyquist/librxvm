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

#include <stdint.h>
#include "regexvm_err.h"

#ifndef REGEXVM_COMMON_H_
#define REGEXVM_COMMON_H_

#define CHARC_BLOCK_SIZE      10

typedef struct stackitem stackitem_t;
typedef struct inst inst_t;
typedef struct stack stack_t;
typedef struct context context_t;

/*  one node in a list */
struct stackitem {
    void *data;
    stackitem_t *next;
    stackitem_t *previous;
};

/* a list */
struct stack {
    stackitem_t *head;
    stackitem_t *tail;
    stackitem_t *dangling_alt;
    int size;
    int dsize;
};

/* VM instruction types */
enum {
    OP_CHAR, OP_ANY, OP_CLASS, OP_BRANCH, OP_JMP, OP_SOL, OP_EOL, OP_MATCH
};

/* instruction */
struct inst {
    char *ccs;
    int x;
    int y;
    char c;
    uint8_t op;
};

struct context {
    char *ccs;
    stack_t *target;
    stack_t *buf;
    stack_t *prog;
    stackitem_t *operand;
    stack_t *parens;
    char *simple;
    int tok;
    int lasttok;
    unsigned int clen;
    unsigned int cspace;
    uint8_t chained;
};

#endif
