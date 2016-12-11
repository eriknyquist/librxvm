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

#ifndef _VMCODE_H_
#define _VMCODE_H_

stackitem_t *stack_add_inst_head (stack_t *stack, inst_t *inst);
void attach_dangling_alt (context_t *cp);

int code_match (context_t *cp);
int code_ccs (context_t *cp, uint8_t is_nchar);
int code_one (context_t *cp, unsigned int size, stackitem_t *i);
int code_zero (context_t *cp, unsigned int size, stackitem_t *i);
int code_onezero (context_t *cp, unsigned int size, stackitem_t *i);
int code_alt (context_t *cp, stackitem_t *i);
int code_rep_n (context_t *cp, int rep_n, stackitem_t *i);
int code_rep_less (context_t *cp, int rep_m, unsigned int size, stackitem_t *i);
int code_rep_more (context_t *cp, int rep_n, unsigned int size, stackitem_t *i);
int code_rep_range (context_t *cp, int rep_n, int rep_m, unsigned int size,
                    stackitem_t *i);

#endif
