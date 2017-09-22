#include <stdint.h>
#include "rxvm_err.h"

#ifndef RXVM_COMMON_H_
#define RXVM_COMMON_H_

#define CHARC_BLOCK_SIZE      10

typedef struct stackitem stackitem_t;
typedef struct inst inst_t;
typedef struct ir_stack ir_stack_t;
typedef struct context context_t;

/*  one node in a list */
struct stackitem {
    void *data;
    stackitem_t *next;
    stackitem_t *previous;
};

/* a list */
struct ir_stack {
    stackitem_t *head;
    stackitem_t *tail;
    stackitem_t *dangling_alt;
    int size;
    int dsize;
};

/* VM instruction types */
enum {
    OP_CHAR, OP_ANY, OP_CLASS, OP_NCLASS, OP_BRANCH, OP_JMP, OP_SOL, OP_EOL,
    OP_MATCH
};

/* instruction */
struct inst {
    char *ccs;
    int x;
    int y;
    char c;
    uint8_t op;
};

#endif
