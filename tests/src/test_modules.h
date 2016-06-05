#ifndef TEST_MODULES_H_
#define TEST_MODULES_H_

int test_regexvm_compile (int *count);
int test_regexvm_match (int *count);
int test_regexvm_search (int *count);
int test_regexvm_err (int *count);
int test_fuzz_regexvm_match (int *count);

#endif
