#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regexvm.h"
#include "lex.h"
#include "test_common.h"

char *gen_randinput (randinput_cfg_t *cfg, uint64_t *len)
{
    inst_t **exe;
    inst_t *inst;
    char rand;
    unsigned int ip;
    unsigned int ix;
    strb_t strb;

    cfg->strb = &strb;
    strb_init(cfg->strb, 50);
    ip = 0;

    exe = cfg->compiled->exe;
    inst = exe[ip];
    while (inst->op != OP_MATCH) {
        switch (inst->op) {
            case OP_CHAR:
                strb_addc(cfg->strb, inst->c);
                ++ip;
            break;
            case OP_ANY:
                rand = (char) rand_range(PRINTABLE_LOW, PRINTABLE_HIGH);
                strb_addc(cfg->strb, rand);
                ++ip;
            break;
            case OP_SOL:
                if (cfg->strb->size &&
                        cfg->strb->buf[cfg->strb->size - 1] != '\n') {
                    strb_addc(cfg->strb, '\n');
                }
                ++ip;
            break;
            case OP_EOL:
                if (cfg->compiled->size > ip && (exe[ip + 1] != OP_CHAR
                            || exe[ip + 1]->c != '\n')) {
                    strb_addc(cfg->strb, '\n');
                }
                ++ip;
            break;
            case OP_CLASS:
                ix = rand_range(0, strlen(inst->ccs) - 1);
                strb_addc(cfg->strb, inst->ccs[ix]);
                ++ip;
            break;
            case OP_BRANCH:
                ip = (rand_range(0, 1)) ? inst->x : inst->y;
            break;
            case OP_JMP:
                ip = inst->x;
            break;
        }

        inst = exe[ip];
    }

    if (len) {
        *len = cfg->strb->size;
    }

    strb_addc(cfg->strb, '\0');
    return cfg->strb->buf;
}
