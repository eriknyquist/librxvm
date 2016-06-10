#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#define NUM_TESTS_COMPILE      18
#define NUM_TESTS_ERR          31
#define NUM_TESTS_ITER         19
#define NUM_TESTS_MATCH        23
#define NUM_TESTS_NOMATCH      5
#define NUM_TESTS_FUZZ_MATCH   20

#define NUM_TESTS              (NUM_TESTS_COMPILE + NUM_TESTS_MATCH + \
                                NUM_TESTS_ITER + NUM_TESTS_ERR + \
                                NUM_TESTS_FUZZ_MATCH)
#define EXABYTES                (1024ULL * 1024ULL * 1024ULL * 1024ULL * \
                                1024ULL * 1024ULL)

typedef struct results results_t;
typedef int(*testmod_t)(int *);

char *generate_matching_string (regexvm_t *compiled);
int compile_testexp (regexvm_t *compiled, char *exp);
void regexvm_print_oneline (regexvm_t *compiled);
void regexvm_print_err (int err);
char *hrsize (uint64_t size);

#endif
