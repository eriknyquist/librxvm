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
#include "stage1.h"
#include "stage2.h"
#include "stack.h"
#include "vm.h"

#define FBUF_SIZE  256

static char *fbuf;
static int bufpos;
static long fpos;
static uint64_t *charcp;

int rxvm_compile (rxvm_t *compiled, char *exp)
{
    stack_t *ir;
    int ret;
    size_t size;

    if (!compiled || !exp)              return RXVM_EPARAM;
    if ((ret = stage1(exp, &ir)) < 0)   return ret;

    if (ir == NULL) {
        size = sizeof(char) * (strlen(exp) + 1);
        if ((compiled->simple = malloc(size)) == NULL) {
            return RXVM_EMEM;
        }

        memcpy(compiled->simple, exp, size);
        compiled->exe = NULL;
        return 0;
    }

    if ((ret = stage2(ir, compiled)) < 0) return ret;
    return 0;
}

char getchar_str (void *data)
{
    char **input;

    input = (char **)data;
    return *(*input)++;
}

static int simple_match (threads_t *tm, char *simple)
{
    char c;
    char *orig;

    orig = simple;
    tm->match_start = tm->chars;

    while (*simple) {
        c = (*tm->getchar)(tm->getchar_data);
        ++tm->chars;

        if (c == tm->endchar)
            return 1;

        if (*simple == '\\')
            ++simple;

skip_readchar:
        if (!char_match(tm->icase, *simple, c)) {
            if (!tm->search) return 0;

            tm->match_start = tm->chars;

            if (simple != orig) {
                simple = orig;
                --tm->match_start;
                goto skip_readchar;
            }
        } else {
            ++simple;
        }
    }

    tm->match_end = tm->chars + 1;
    return 1;
}

int rxvm_search (rxvm_t *compiled, char *input, char **start, char **end,
                 int flags)
{
    char *sot;
    threads_t tm;
    int ret;

    if (!compiled || !input) return RXVM_EPARAM;

    memset(&tm, 0, sizeof(threads_t));
    tm.getchar = getchar_str;
    tm.getchar_data = &input;
    tm.endchar = '\0';

    sot = input;
    tm.icase = (flags & RXVM_ICASE);
    tm.nongreedy = (flags & RXVM_NONGREEDY);
    tm.multiline = (flags & RXVM_MULTILINE);
    tm.search = 1;

    ret = 0;
    if (compiled->simple) {
        simple_match(&tm, compiled->simple);
    } else {
        if ((ret = vm_init(&tm, compiled->size)) != 0)
            goto cleanup;

        vm_execute(&tm, compiled);
    }

    if (tm.match_end == 0) {
        if (start)
            *start = NULL;
        if (end)
            *end = NULL;
    } else {
        if (start)
            *start = sot + tm.match_start;
        if (end)
            *end = sot + (tm.match_end - 1);
        ret = 1;
    }

cleanup:
    vm_cleanup(&tm);
    return ret;
}

int rxvm_match (rxvm_t *compiled, char *input, int flags)
{
    char *sot;
    threads_t tm;
    int ret;

    if (!compiled || !input) return RXVM_EPARAM;

    memset(&tm, 0, sizeof(threads_t));
    tm.getchar = getchar_str;
    tm.getchar_data = &input;
    tm.endchar = '\0';

    sot = input;
    tm.multiline = 1;
    tm.icase = (flags & RXVM_ICASE);
    tm.nongreedy = (flags & RXVM_NONGREEDY);

    ret = 0;
    if (compiled->simple) {
        if (!simple_match(&tm, compiled->simple)) {
            goto cleanup;
        }
    } else {
        if ((ret = vm_init(&tm, compiled->size)) != 0)
            goto cleanup;

        if (vm_execute(&tm, compiled))
            goto cleanup;
    }

    ret = tm.match_end && !*(sot + (tm.match_end - 1));

cleanup:
    vm_cleanup(&tm);
    return ret;
}

#ifndef NOEXTRAS
static char getchar_file (void *data)
{
    size_t num;
    FILE *fp;

    fp = (FILE *)data;

    if (bufpos == FBUF_SIZE) {
        bufpos = 0;
        num = fread(fbuf, sizeof(char), FBUF_SIZE, fp);
        fpos += FBUF_SIZE;

        if (num < FBUF_SIZE) {
            fbuf[num] = EOF;
        }
    }

    return fbuf[bufpos++];
}

int rxvm_fsearch (rxvm_t *compiled, FILE *fp, uint64_t *match_size,
                  int flags)
{
    threads_t tm;
    int ret;
    uint64_t msize;
    uint64_t start;
    uint64_t seek_size;
    char file_buffer[FBUF_SIZE];

    if (!compiled || !fp) return RXVM_EPARAM;
    if ((fpos = ftell(fp)) < 0) return RXVM_IOERR;

    memset(&tm, 0, sizeof(threads_t));
    tm.getchar = getchar_file;
    tm.getchar_data = fp;
    tm.endchar = EOF;

    fbuf = file_buffer;
    bufpos = FBUF_SIZE;
    tm.icase = (flags & RXVM_ICASE);
    tm.nongreedy = (flags & RXVM_NONGREEDY);
    tm.multiline = (flags & RXVM_MULTILINE);
    tm.search = 1;
    charcp = &tm.chars;
    start = fpos;

    ret = 0;
    if (compiled->simple) {
        while (!simple_match(&tm, compiled->simple));
    } else {
        if ((ret = vm_init(&tm, compiled->size)) != 0)
            goto cleanup;

        vm_execute(&tm, compiled);
    }

    if (tm.match_end) {
        msize = tm.match_end - tm.match_start - 1;
        seek_size = (start + tm.chars) - msize - (compiled->simple == NULL);

        if (match_size) *match_size = msize;

        fseek(fp, seek_size, SEEK_SET);
        ret = 1;
    }

cleanup:
    vm_cleanup(&tm);
    return ret;
}

void rxvm_print (rxvm_t *compiled)
{
    unsigned int i;
    inst_t inst;

    if (!compiled) return;

    if (compiled->simple) {
        printf("\"%s\" no compilation required\n", compiled->simple);
        return;
    }

    for (i = 0; i < compiled->size; i++) {
        inst = compiled->exe[i];

        switch(inst.op) {
            case OP_CHAR:
                printf("%d\tchar %c\n", i, inst.c);
            break;
            case OP_ANY:
                printf("%d\tany\n", i);
            break;
            case OP_SOL:
                printf("%d\tsol\n", i);
            break;
            case OP_EOL:
                printf("%d\teol\n", i);
            break;
            case OP_CLASS:
                printf("%d\tclass %s\n", i, inst.ccs);
            break;
            case OP_BRANCH:
                printf("%d\tbranch %d %d\n", i, inst.x, inst.y);
            break;
            case OP_JMP:
                printf("%d\tjmp %d\n", i, inst.x);
            break;
            case OP_MATCH:
                printf("%d\tmatch\n", i);
            break;
        }
    }
}
#endif /* NOEXTRAS */

void rxvm_free (rxvm_t *compiled)
{
    unsigned int i;

    if (!compiled) return;

    if (compiled->simple) {
        free(compiled->simple);
        return;
    } else if (compiled->exe == NULL) {
        return;
    }

    for (i = 0; i < compiled->size; i++) {
        if (compiled->exe[i].ccs != NULL)
            free(compiled->exe[i].ccs);
    }

    free(compiled->exe);
}
