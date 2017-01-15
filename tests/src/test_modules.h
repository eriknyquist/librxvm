#ifndef TEST_MODULES_H_
#define TEST_MODULES_H_

int test_rxvm_compile (int *count);
int test_rxvm_lfix_heuristic (int *count);
int test_rxvm_match (int *count);
int test_rxvm_search (int *count);
int test_rxvm_search_multi (int *count);
int test_rxvm_search_nomatch (int *count);
int test_rxvm_err (int *count);
int test_fuzz_rxvm_match (int *count);
int test_fuzz_full_rxvm_match (int *count);

#endif
