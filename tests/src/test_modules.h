#ifndef TEST_MODULES_H_
#define TEST_MODULES_H_

int test_regexvm_compile (int *count);
int test_regexvm_match (int *count);
int test_regexvm_iter (int *count);
int test_regexvm_err (int *count);
int fuzz_regexvm_match (int *count);

#endif
