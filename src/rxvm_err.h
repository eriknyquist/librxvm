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

#ifndef RXVM_ERR_H_
#define RXVM_ERR_H_

#define RXVM_BADOP      -1   /* Operator used incorrectly */
#define RXVM_BADCLASS   -2   /* Unexpected character class closing character */
#define RXVM_BADREP     -3   /* Unexpected closing repetition character */
#define RXVM_BADPAREN   -4   /* Unexpected closing parenthesis */
#define RXVM_EPAREN     -5   /* Unterminated parenthesis group */
#define RXVM_ECLASS     -6   /* Unterminated character class */
#define RXVM_EREP       -7   /* Missing repetition closing character */
#define RXVM_MREP       -8   /* Empty repetition */
#define RXVM_ETRAIL     -9   /* Trailing escape character */
#define RXVM_EMEM       -10  /* Failed to allocate memory */
#define RXVM_EINVAL     -11  /* Invalid symbol */
#define RXVM_EPARAM     -12  /* Invalid parameter passed to library function */

#endif
