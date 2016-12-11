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
#include "rxvm.h"
#include "lex.h"
#include "vm.h"
#include "rxvm_common.h"

#define BUFSIZE 8192
#define ASIZE (PRINTABLE_HIGH - WS_LOW)
#define PSIZE (PRINTABLE_HIGH - PRINTABLE_LOW)
#define WSIZE (WS_HIGH - WS_LOW)

#define atoindex(a) (a - WS_LOW)
#define indextoa(i) (i + WS_LOW)

static unsigned int badchar[ASIZE];
static char bmhbuf[BUFSIZE];

static FILE *fp;
static char *pattern;
static unsigned int plen;
static unsigned int pos;
static unsigned int end;
static unsigned int endix;
static unsigned int pendix;

static void fill (unsigned int backup)
{
    unsigned int i, rsize;

    memmove(bmhbuf, bmhbuf + (end - backup), backup);

    if ((rsize = fread(bmhbuf + backup,
                       sizeof(char),
                       BUFSIZE - backup,
                       fp)) < (BUFSIZE - backup)) {

        memset(bmhbuf + backup + rsize, EOF, BUFSIZE - (backup + rsize));
        end = rsize;
    } else {
        end = BUFSIZE;
    }

    endix = end - 1;
}

static char skip (unsigned int num)
{
    unsigned int total;

    total = pos + num;

    if (total > endix) {
        fill(plen - (total - endix));
        pos = pendix;
    } else {
        pos += num;
    }

    return bmhbuf[pos];
}

int bmh (threads_t *tm)
{
    char c;
    unsigned int i, skiplen;
    long fpos;
    char *input;

    skiplen = pendix;

skip:
    i = pendix;
    while ((c = skip(skiplen)) != EOF) {
        input = bmhbuf + (pos - pendix);
        do {
            if (!char_match(tm->icase, input[i], pattern[i])) {
                skiplen = (isprintable(c)) ? badchar[atoindex(c)] : plen;
                goto skip;
            }
        } while (i--);

        fpos = ftell(fp);
        tm->match_start = fpos - 1 - pendix - (endix - pos);
        tm->match_end = fpos + 1 - (endix - pos);

        return 1;
    }

    return 0;
}

void bmh_init (FILE *file, char *pat, unsigned int length)
{
    unsigned int i;

    for (i = 0; i < ASIZE; ++i) {
        badchar[i] = length;
    }

    for (i = 0; i < length - 1; ++i) {
        badchar[atoindex(pat[i])] = length - i - 1;
    }

    pos = 0;
    pattern = pat;
    fp = file;
    plen = length;
    pendix = plen - 1;

    fill(0);
}
