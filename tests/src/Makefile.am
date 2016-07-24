gen_randinputdir = tests/src
gen_randexpdir = tests/src

check_PROGRAMS = regexvm_test
gen_randinput_PROGRAMS = gen_randinput
gen_randexp_PROGRAMS = gen_randexp
TESTS = regexvm_test

gen_randexp_SOURCES = gen_randexp.c randexp.c string_builder.c test_utils.c
gen_randinput_SOURCES = gen_randinput.c randinput.c string_builder.c test_utils.c

regexvm_test_SOURCES = test_run.c test_regexvm_compile.c test_regexvm_err.c \
test_regexvm_search.c test_regexvm_match.c test_fuzz_regexvm_match.c \
test_fuzz_full_regexvm_match.c test_regexvm_search_nomatch.c test_utils.c \
randinput.c randexp.c string_builder.c

LDADD = -lregexvm -L../../src
AM_CFLAGS = -I../../src -Wall -Wno-trigraphs
AM_DEFAULT_SOURCE_EXT = .c
