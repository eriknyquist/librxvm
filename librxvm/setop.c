#include <string.h>
#include "rxvm_common.h"

void set_op_class (inst_t *inst, char *ccs)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_CLASS;
    inst->ccs = ccs;
}

void set_op_nclass (inst_t *inst, char *ccs)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_NCLASS;
    inst->ccs = ccs;
}

void set_op_branch (inst_t *inst, int x, int y)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_BRANCH;
    inst->x = x;
    inst->y = y;
}

void set_op_jmp (inst_t *inst, int x)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_JMP;
    inst->x = x;
}

void set_op_match (inst_t *inst)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_MATCH;
}

void set_op_char (inst_t *inst, char c)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_CHAR;
    inst->c = c;
}

void set_op_any (inst_t *inst)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_ANY;
}

void set_op_sol (inst_t *inst)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_SOL;
}

void set_op_eol (inst_t *inst)
{
    memset(inst, 0, sizeof(inst_t));
    inst->op = OP_EOL;
}
