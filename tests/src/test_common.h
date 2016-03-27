#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#define NUM_TESTS_COMPILE      10
#define NUM_TESTS_ERR          22
#define NUM_TESTS_ITER         19
#define NUM_TESTS_MATCH        17

#define NUM_TESTS              (NUM_TESTS_COMPILE + NUM_TESTS_MATCH + \
                                NUM_TESTS_ITER + NUM_TESTS_ERR)

typedef struct results results_t;
typedef int(*testmod_t)(int *);

struct results {
    unsigned int passed;
    unsigned int failed;
};

#endif
