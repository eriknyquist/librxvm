#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#define NUM_TESTS_COMPILE      11
#define NUM_TESTS_ERR          22
#define NUM_TESTS_ITER         19
#define NUM_TESTS_MATCH        17
#define NUM_TESTS_FUZZ_MATCH   4

#define NUM_TESTS              (NUM_TESTS_COMPILE + NUM_TESTS_MATCH + \
                                NUM_TESTS_ITER + NUM_TESTS_ERR + \
                                NUM_TESTS_FUZZ_MATCH)

typedef struct results results_t;
typedef int(*testmod_t)(int *);

char *generate_matching_string (regexvm_t *compiled);
int compile_testexp (regexvm_t *compiled, char *exp);

#endif
