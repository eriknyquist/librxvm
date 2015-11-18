#ifndef LEX_H_
#define LEX_H_

#define CHARC_OPEN_SYM       '['
#define CHARC_CLOSE_SYM      ']'
#define RANGE_SEP_SYM        '-'
#define ONE_SYM              '+'
#define ZERO_SYM             '*'
#define ANY_SYM              '.'
#define DEREF_SYM            '\\'

enum {CHARC_OPEN, CHARC_CLOSE, CHAR_RANGE, ONE,
      ZERO, ANY, LITERAL, INVALIDSYM, END};
enum {STATE_START, STATE_LITERAL, STATE_RANGE, STATE_CLASS, STATE_DEREF,
      STATE_END};
int lex (char **input);
char *get_token_text (void);

#endif
