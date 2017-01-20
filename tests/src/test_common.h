#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#include "string_builder.h"

#define TEST_TRS                   "rxvm_test.trs"
#define TEST_LOG                   "rxvm_test.log"
#define NUM_TESTS_COMPILE          48
#define NUM_TESTS_ERR              35
#define NUM_TESTS_SEARCH           20
#define NUM_TESTS_SEARCH_MULTI     20
#define NUM_TESTS_MATCH            25
#define NUM_TESTS_NOMATCH          5
#define NUM_TESTS_FUZZ_MATCH       29
#define NUM_TESTS_FUZZ_FULL_MATCH  10

#define EXABYTES                (1024ULL * 1024ULL * 1024ULL * 1024ULL * \
                                1024ULL * 1024ULL)

typedef struct results results_t;
typedef int(*testmod_t)(int *);
typedef struct randexp_cfg randexp_cfg_t;

struct randexp_cfg {
    strb_t *strb;
    uint8_t literals;   /* 0-100; higher means more literals */
    uint8_t escapes;    /* 0-100; higher means more escaped literals */
    uint8_t tokens;     /* 0-100; higher means more tokens */

    uint64_t limit;
};

void init_charmap(void);
int compile_testexp (rxvm_t *compiled, char *exp);
unsigned int rand_range (unsigned int low, unsigned int high);

char *gen_randexp (randexp_cfg_t *cfg, uint64_t *len);

void test_err (char *regex, char *input, const char *test, char *msg, int ret);
void rxvm_print_oneline (rxvm_t *compiled);
void hrsize (uint64_t size, char *buf, unsigned int bufsize);

extern FILE *logfp;
extern FILE *trsfp;

#endif
