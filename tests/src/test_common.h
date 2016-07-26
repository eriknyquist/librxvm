#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#include "string_builder.h"

#define NUM_TESTS_COMPILE          18
#define NUM_TESTS_ERR              31
#define NUM_TESTS_ITER             19
#define NUM_TESTS_MATCH            23
#define NUM_TESTS_NOMATCH          5
#define NUM_TESTS_FUZZ_MATCH       20
#define NUM_TESTS_FUZZ_FULL_MATCH  20

#define NUM_TESTS              (NUM_TESTS_COMPILE + NUM_TESTS_MATCH + \
                                NUM_TESTS_ITER + NUM_TESTS_ERR + \
                                NUM_TESTS_FUZZ_MATCH)
#define EXABYTES                (1024ULL * 1024ULL * 1024ULL * 1024ULL * \
                                1024ULL * 1024ULL)

typedef struct results results_t;
typedef int(*testmod_t)(int *);
typedef struct randexp_cfg randexp_cfg_t;
typedef struct randinput_cfg randinput_cfg_t;

struct randexp_cfg {
    strb_t *strb;
    uint8_t literals;   /* 0-100; higher means more literals */
    uint8_t escapes;    /* 0-100; higher means more escaped literals */
    uint8_t tokens;     /* 0-100; higher means more tokens */
};

struct randinput_cfg {
    uint8_t generosity;
    uint8_t whitespace;
};

void init_charmap(void);
int compile_testexp (regexvm_t *compiled, char *exp);
unsigned int rand_range (unsigned int low, unsigned int high);

char *gen_randinput (regexvm_t *compiled, randinput_cfg_t *cfg);
char *gen_randexp (randexp_cfg_t *cfg, uint64_t *len);

void test_err (char *regex, char *input, const char *test, char *msg, int ret);
void regexvm_print_oneline (regexvm_t *compiled);
void regexvm_print_err (int err);
char *hrsize (uint64_t size);

#endif
