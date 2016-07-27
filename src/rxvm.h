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

#ifndef RXVM_H_
#define RXVM_H_

#include "rxvm_common.h"

/* Config. flags */
#define RXVM_ICASE           0x1
#define RXVM_NONGREEDY       0x2
#define RXVM_MULTILINE       0x4

typedef struct rxvm_gencfg rxvm_gencfg_t;
typedef struct rxvm rxvm_t;

struct rxvm {
    inst_t **exe;
    unsigned int size;
    char *simple;
};

struct rxvm_gencfg {
    uint8_t generosity;
    uint8_t whitespace;
};

char *rxvm_gen   (rxvm_t *compiled, rxvm_gencfg_t *cfg);
int rxvm_compile (rxvm_t *compiled, char *exp);
int rxvm_match   (rxvm_t *compiled, char *input, int flags);
int rxvm_fsearch (rxvm_t *compiled, FILE *fp, uint64_t *match_size, int flags);
int rxvm_search  (rxvm_t *compiled, char *input, char **start, char **end,
                  int flags);

void rxvm_free   (rxvm_t *compiled);
void rxvm_print  (rxvm_t *compiled);

#endif
