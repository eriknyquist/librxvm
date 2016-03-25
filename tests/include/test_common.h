#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

typedef struct results results_t;
typedef int(*testmod_t)(results_t *);

struct results {
    unsigned int passed;
    unsigned int failed;
};

#endif
