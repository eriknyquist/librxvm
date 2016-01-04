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
#define DEREF_SYM            '\\'

/* lexer return tokens */
enum {CHARC_OPEN, CHARC_CLOSE, CHAR_RANGE, LPAREN, RPAREN, ONE,
      ZERO, ONEZERO, ANY, LITERAL, INVALIDSYM, END};

/* internal lexer states */
enum {STATE_START, STATE_LITERAL, STATE_RANGE, STATE_CLASS, STATE_DEREF,
      STATE_END};

int lex (char **input);
char *get_token_text (void);

#endif
