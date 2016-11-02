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

#ifndef VM_H_
#define VM_H_

typedef struct threads threads_t;

struct threads {
    uint64_t chars;         /* Number of input chars processed */
    uint64_t match_start;   /* Start of matching portion of input stream */
    uint64_t match_end;     /* End of matching portion of input stream */
    int *cp;                /* Threads for current input char. */
    int *np;                /* Threads for next input char. */
    uint8_t *cp_lookup;     /* Lookup table for current input char. */
    uint8_t *np_lookup;     /* Lookup table for next input char. */
    uint8_t *table_base;    /* Base address for memory area containing tables */
    char (*getchar)(void*); /* Ptr to function for getting next input char */
    void *getchar_data;     /* Data for getchar function */
    char endchar;           /* The char that signals the end of input */
    char lastinput;         /* Save previous input char to allow back-peek */
    int csize;              /* No. of threads queued for current input char. */
    int nsize;              /* No. of threads queued for next input. char. */

    /* Flags */
    uint8_t icase;
    uint8_t nongreedy;
    uint8_t multiline;
};

int vm_init (threads_t *tm, unsigned int size);
int vm_execute (threads_t *tm, rxvm_t *compiled);
void vm_cleanup (threads_t *tm);
int char_match (uint8_t icase, char a, char b);

#endif
