#ifndef COMMON_H_
#define COMMON_H_

#define MAX_CHARC_LEN          256
#define MAX_NEST_PARENS        64

typedef struct stackitem stackitem_t;
typedef struct inst inst_t;
typedef struct stack stack_t;

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
typedef struct inst {
    int op;
    int num;
    char c;
    char *ccs;
    stackitem_t *x;
    stackitem_t *y;
} inst_t;

#endif
