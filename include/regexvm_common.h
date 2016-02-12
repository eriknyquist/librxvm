#ifndef REGEXVM_COMMON_H_
#define REGEXVM_COMMON_H_

#if (MAX_CHARC_LEN)
#define MAXCHARCLEN           MAX_CHARC_LEN
#else
#define MAXCHARCLEN           512
#endif

#if (MAX_NEST_PARENS)
#define MAXNESTPARENS         MAX_NEST_PARENS
#else
#define MAXNESTPARENS         512
#endif

#include <stdint.h>
#include "regexvm_err.h"

typedef struct stackitem stackitem_t;
typedef struct inst inst_t;
typedef struct stack stack_t;
typedef struct context context_t;

/*  one node in a list */
struct stackitem {
    inst_t *inst;
    stackitem_t *next;
    stackitem_t *previous;
};

/* a list */
struct stack {
    stackitem_t *head;
    stackitem_t *tail;
    stackitem_t *dangling_alt;
    unsigned short size;
    unsigned short dsize;
};

/* VM instruction types */
enum {
    OP_CHAR, OP_ANY, OP_CLASS, OP_BRANCH, OP_JMP, OP_MATCH
};

/* instruction */
struct inst {
    char *ccs;
    short x;
    short y;
    char c;
    uint8_t op;
};

struct context {
    stack_t *target;
    stack_t *buf;
    stack_t *prog;
    stackitem_t *operand;
    stack_t *parens[MAXNESTPARENS + 1];
    int tok;
    int lasttok;
    unsigned int pdepth;
    unsigned int hdepth;
    unsigned int clen;
};

#endif
