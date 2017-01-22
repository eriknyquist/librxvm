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

static unsigned int ltemp0;
static unsigned int ltempn;
static unsigned int lfix0;
static unsigned int lfixn;
static unsigned int lfixs;

void lfix_zero (void)
{
    ltemp0 = ltempn = lfix0 = lfixn = 0;
}

void lfix_start (char *orig, char *lp1)
{
    lfixs = 1;
    ltemp0 = ltempn = lp1 - orig;
}

void lfix_continue (char *orig, char *lp1)
{
    if (lfixs) ++ltempn;
    else lfix_start(orig, lp1);
}

unsigned int lfix_longest (void)
{
    return lfixn - lfix0;
}

unsigned int lfix_current (void)
{
    return ltempn - ltemp0;
}

void lfix_set_longest (void)
{
    lfix0 = ltemp0;
    lfixn = ltempn;
}

void lfix_update_longest (void)
{
    if (lfix_current() > lfix_longest()) {
        lfix_set_longest();
    }
}

void lfix_stop (void)
{
    lfix_update_longest();
    lfixs = ltemp0 = ltempn = 0;
}

void lfix_dec (void)
{
    if (ltempn > ltemp0) --ltempn;
}

unsigned int get_lfix0 (void)
{
    return lfix0;
}

unsigned int get_lfixn (void)
{
    return lfixn;
}

void set_lfixn (unsigned int val)
{
    lfixn = val;
}

void set_ltempn (unsigned int val)
{
    ltempn = val;
}

unsigned int get_ltempn (void)
{
    return ltempn;
}
