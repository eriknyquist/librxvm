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

#ifndef REGEXVM_H_
#define REGEXVM_H_

#include "regexvm_common.h"

/* Config. flags */
#define REGEXVM_ICASE           0x1
#define REGEXVM_NONGREEDY       0x2
#define REGEXVM_MULTILINE       0x4

typedef struct regexvm regexvm_t;

struct regexvm {
    inst_t **exe;
    unsigned int size;
};

int regexvm_compile (regexvm_t *compiled, char *exp);
int regexvm_match (regexvm_t *compiled, char *input, int flags);
int regexvm_fsearch (regexvm_t *compiled, FILE *fp, uint64_t *match_size,
                     int flags);
int regexvm_search (regexvm_t *compiled, char *input, char **start, char **end,
                  int flags);

void regexvm_free (regexvm_t *compiled);
void regexvm_print (regexvm_t *compiled);

#endif
