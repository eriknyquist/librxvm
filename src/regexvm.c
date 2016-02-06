#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"
#include "stage1.h"
#include "stage2.h"
#include "stack.h"

int regexvm_compile (regexvm_t *compiled, char *exp)
{
    stack_t *ir;
    int ret;

    if ((ret = stage1(exp, &ir)) < 0)
        return ret;

    if ((ret = stage2(ir, compiled)) < 0)
        return ret;

    return 0;
}

void regexvm_print (regexvm_t *compiled)
{
    unsigned int i;
    inst_t *inst;

    for (i = 0; i < compiled->size; i++) {
        inst = compiled->exe[i];

        switch(inst->op) {
            case OP_CHAR:
                printf("%d\tchar %c\n", i, inst->c);
            break;
            case OP_ANY:
                printf("%d\tany\n", i);
            break;
            case OP_CLASS:
                printf("%d\tclass %s\n", i, inst->ccs);
            break;
            case OP_BRANCH:
                printf("%d\tbranch %d %d\n", i, inst->x, inst->y);
            break;
            case OP_JMP:
                printf("%d\tjmp %d\n", i, inst->x);
            break;
            case OP_MATCH:
                printf("%d\tmatch\n", i);
            break;
        }
    }
}

void regexvm_free (regexvm_t *compiled)
{
    unsigned int i;
    inst_t *inst;

    for (i = 0; i < compiled->size; i++) {
        inst = compiled->exe[i];

        if (inst->ccs != NULL)
            free(inst->ccs);

        free(inst);
    }

    free(compiled->exe);
}

void regexvm_print_err (int err)
{
    const char *msg;

    switch (err) {
        case RVM_BADOP:
            msg = "Operator used incorrectly";
        break;
        case RVM_BADCLASS:
            msg = "Unexpected character class closing character";
        break;
        case RVM_BADPAREN:
            msg = "Unexpected parenthesis group closing character";
        break;
        case RVM_EPAREN:
            msg = "Unterminated parenthesis group";
        break;
        case RVM_ECLASS:
            msg = "Unterminated character class";
        break;
        case RVM_ETRAIL:
            msg = "Trailing escape character";
        break;
        case RVM_EMEM:
            msg = "Failed to allocate memory";
        break;
        case RVM_ENEST:
            msg = "Too many nested parenthesis groups";
        break;
        case RVM_ECLASSLEN:
            msg = "Too many elements in character class";
        break;
        case RVM_EINVAL:
            msg = "Unrecognised symbol";
        break;
        default:
            msg = "Unrecognised error code";
    }

    printf("Error %d: %s\n", err, msg);
}
