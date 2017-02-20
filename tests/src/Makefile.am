randexpdir = tests/src
check_PROGRAMS = rxvm_test
randexp_PROGRAMS = randexp
TESTS = $(check_PROGRAMS)
LOG_DRIVER = ./$(check_PROGRAMS)

randexp_SOURCES = gen_randexp.c randexp.c
rxvm_test_SOURCES = test_run.c test_rxvm_compile.c test_rxvm_err.c \
test_rxvm_search.c test_rxvm_match.c test_rxvm_search_nomatch.c \
test_rxvm_search_multi.c test_rxvm_api.c test_utils.c randexp.c

LDADD = -lrxvm -L../../librxvm
AM_CFLAGS = -I../../librxvm -Wall -Wno-trigraphs
AM_DEFAULT_SOURCE_EXT = .c

if NOEXTRAS
AM_CFLAGS += -DNOEXTRAS
else
rxvm_test_SOURCES += test_rxvm_fsearch.c test_rxvm_lfix_heuristic.c \
test_rxvm_lfix_to_str.c test_fuzz_full_rxvm_match.c test_fuzz_rxvm_match.c
endif
