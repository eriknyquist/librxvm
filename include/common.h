#ifndef COMMON_H_
#define COMMON_H_

#ifdef MAX_CHARC_LEN
#define MAXCHARCLEN          MAX_CHARC_LEN
#else
#define MAXCHARCLEN          512
#endif

#ifdef MAX_NEST_PARENS
#define MAXNESTPARENS        MAX_NEST_PARENS
#else
#define MAXNESTPARENS        512
#endif

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
    unsigned int size;
};

/* VM instruction types */
enum {
    OP_CHAR, OP_ANY, OP_CLASS, OP_BRANCH, OP_JMP, OP_MATCH
};

/* instruction */
struct inst {
    int op;
    int num;
    char c;
    char *ccs;
    int x;
    int y;
};

struct context {
    stack_t *target;
    stack_t *buf;
    stack_t *prog;
    stackitem_t *operand;
    stackitem_t *dangling_cat;
    int tok;
    int lasttok;
    stack_t *parens[MAXNESTPARENS];
    unsigned int ddepth;
    unsigned int pdepth;
    unsigned int clen;
};

#endif
