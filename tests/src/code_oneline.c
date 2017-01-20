#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"

char *rgx;

void rxvm_print_oneline (rxvm_t *compiled)
{
    unsigned int i;
    inst_t *inst;

    if (compiled->simple) {
        printf("p");
        printf("%s\n", compiled->simple);
        return;
    }

    for (i = 0; i < compiled->size; i++) {
        inst = &compiled->exe[i];

        switch(inst->op) {
            case OP_CHAR:
                printf("l%c", inst->c);
            break;
            case OP_ANY:
                printf("a");
            break;
            case OP_SOL:
                printf("s");
            break;
            case OP_EOL:
                printf("e");
            break;
            case OP_CLASS:
                printf("c%s", inst->ccs);
            break;
            case OP_NCLASS:
                printf("n%s", inst->ccs);
            break;
            case OP_BRANCH:
                printf("b%d,%d", inst->x, inst->y);
            break;
            case OP_JMP:
                printf("j%d", inst->x);
            break;
            case OP_MATCH:
                printf("m");
            break;
        }

        if (i == (compiled->size - 1)) {
            printf("\n");
        } else {
            printf(":");
        }
    }
}

int main (int argc, char *argv[])
{
    int ret;
    rxvm_t compiled;

    ret = 0;
    if (argc != 2) {
        printf("Usage: %s <regex>\n", argv[0]);
        exit(1);
    }

    /* Compile the expression */
    if ((ret = rxvm_compile(&compiled, argv[1])) < 0) {
        rxvm_print_err(ret);
        exit(ret);
    }

    rxvm_print_oneline(&compiled);

    rxvm_free(&compiled);
    return ret;
}
