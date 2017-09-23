#ifndef RXVM_H_
#define RXVM_H_

#include "rxvm_common.h"

/* Config. flags */
#define RXVM_ICASE           0x1
#define RXVM_NONGREEDY       0x2
#define RXVM_MULTILINE       0x4
#define RXVM_SEARCH          0x8

typedef struct rxvm_gencfg rxvm_gencfg_t;
typedef struct rxvm rxvm_t;

struct rxvm {
    inst_t *exe;
    char *simple;
    char *lfix;
    unsigned int lfix0;
    unsigned int lfixn;
    unsigned int size;
};

struct rxvm_gencfg {
    uint8_t generosity;
    uint8_t whitespace;

    uint64_t len;
    uint64_t limit;
};

int rxvm_compile (rxvm_t *compiled, char *exp);
int rxvm_match   (rxvm_t *compiled, char *input, int flags);
int rxvm_search  (rxvm_t *compiled, char *input, char **start, char **end,
                  int flags);
void rxvm_free   (rxvm_t *compiled);

#ifndef NOEXTRAS
void rxvm_print_err (int err);
void rxvm_print  (rxvm_t *compiled);
char *rxvm_gen   (rxvm_t *compiled, rxvm_gencfg_t *cfg);
int rxvm_fsearch (rxvm_t *compiled, FILE *fp, uint64_t *match_size, int flags);
#endif /* NOEXTRAS */

#endif
