regexvmincludedir = $(includedir)/libregexvm
regexvminclude_HEADERS = lex.h regexvm_common.h regexvm_err.h regexvm.h \
						 stack.h stage1.h stage2.h vm.h vmcode.h
lib_LIBRARIES = libregexvm.a
libregexvm_a_SOURCES = stack.c lex.c stage1.c stage2.c vm.c regexvm.c vmcode.c
AM_CFLAGS = -Wall -pedantic
