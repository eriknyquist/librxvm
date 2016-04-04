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

#ifndef LEX_H_
#define LEX_H_

#define PRINTABLE_LOW        ' '  /* ASCII 0x20 */
#define PRINTABLE_HIGH       '~'  /* ASCII 0x7E */
#define WS_LOW               '\a' /* ASCII 0x07 */
#define WS_HIGH              '\r' /* ASCII 0x0D */

#define CHARC_OPEN_SYM       '['
#define CHARC_CLOSE_SYM      ']'
#define LPAREN_SYM           '('
#define RPAREN_SYM           ')'
#define RANGE_SEP_SYM        '-'
#define ONE_SYM              '+'
#define ZERO_SYM             '*'
#define ANY_SYM              '.'
#define ONEZERO_SYM          '?'
#define ALT_SYM              '|'
#define SOL_SYM              '^'
#define EOL_SYM              '$'
#define DEREF_SYM            '\\'

/* lexer return tokens */
enum {CHARC_OPEN, CHARC_CLOSE, CHAR_RANGE, LPAREN, RPAREN, ONE,
      ZERO, ONEZERO, ALT, ANY, LITERAL, SOL, EOL, INVALIDSYM, END};

void lex_init (void);
int lex (char **input);

#endif
