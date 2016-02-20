#ifndef REGEXVM_H_
#define REGEXVM_H_

#include "regexvm_common.h"

typedef struct regexvm regexvm_t;

struct regexvm {
    inst_t **exe;
    unsigned int size;
};

int regexvm_compile (regexvm_t *compiled, char *exp);
int regexvm_match(regexvm_t *compiled, char *input);
void regexvm_free (regexvm_t *compiled);
void regexvm_print (regexvm_t *compiled);

#if (DEBUG)
void regexvm_print_err (int err);
#endif /* DEBUG */

#endif
