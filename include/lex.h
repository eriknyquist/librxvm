#ifndef LEX_H_
#define LEX_H_

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
#define DEREF_SYM            '\\'

/* lexer return tokens */
enum {CHARC_OPEN, CHARC_CLOSE, CHAR_RANGE, LPAREN, RPAREN, ONE,
      ZERO, ONEZERO, ALT, ANY, LITERAL, INVALIDSYM, END};

int lex (char **input);

#endif
