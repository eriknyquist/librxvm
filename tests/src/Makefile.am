check_PROGRAMS = regexvm_test
TESTS = regexvm_test
regexvm_test_SOURCES = run_tests.c test_regexvm_compile.c test_regexvm_err.c \
test_regexvm_iter.c test_regexvm_match.c
LDADD = -lregexvm -L$(libdir)
AM_CFLAGS = -I$(includedir)/libregexvm
AM_DEFAULT_SOURCE_EXT = .ddc
