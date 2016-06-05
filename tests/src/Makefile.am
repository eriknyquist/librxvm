check_PROGRAMS = regexvm_test
TESTS = regexvm_test

regexvm_test_SOURCES = test_run.c test_regexvm_compile.c test_regexvm_err.c \
test_regexvm_search.c test_regexvm_match.c test_fuzz_regexvm_match.c test_utils.c

LDADD = -lregexvm -L../../src
AM_CFLAGS = -I../../src -Wall -Wno-trigraphs
AM_DEFAULT_SOURCE_EXT = .c
