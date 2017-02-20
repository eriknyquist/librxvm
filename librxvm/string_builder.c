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
#include <stdint.h>
#include <string.h>
#include "string_builder.h"
#include "rxvm_err.h"

#define DEFAULT_BLOCK_SIZE     10

static int enlarge_buf (strb_t *cfg, int blocks)
{
    char *temp;
    int size;

    size = cfg->block_size * blocks;
    if ((temp = realloc(cfg->buf, cfg->space + size)) == NULL) {
         return RXVM_EMEM;
    }

    cfg->space += size;
    cfg->buf = temp;
    return 0;
}

int strb_init (strb_t *cfg, unsigned int block_size)
{
    if (!cfg) return RXVM_EPARAM;

    memset(cfg, 0, sizeof(strb_t));
    cfg->block_size = (block_size) ? block_size : DEFAULT_BLOCK_SIZE;

    if ((cfg->buf = malloc(cfg->block_size)) == NULL) {
        return RXVM_EMEM;
    }

    cfg->space = cfg->block_size;
    return 0;
}

int strb_addc (strb_t *cfg, char c)
{
    if ((cfg->size + 1) > cfg->space) {
        if (enlarge_buf(cfg, 1) < 0) {
            return RXVM_EMEM;
        }
    }

    cfg->buf[cfg->size] = c;
    ++cfg->size;
    return 0;
}

int strb_adds (strb_t *cfg, char *s, int n)
{
    int i;
    int blocks;

    if ((cfg->size + n) > cfg->space) {
        blocks = (((cfg->size + n) - cfg->space) / cfg->block_size) + 1;
        if (enlarge_buf(cfg, blocks) < 0) {
            return RXVM_EMEM;
        }
    }

    for (i = 0; i < n && s[i]; ++i) {
        cfg->buf[cfg->size++] = s[i];
    }

    return 0;
}

int strb_addu (strb_t *cfg, unsigned int i)
{
    int chars;
    char num[20];

    chars = snprintf(num, sizeof(num), "%u", i);
    return strb_adds(cfg, num, chars);
}
