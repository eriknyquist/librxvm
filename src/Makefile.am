regexvmincludedir = $(includedir)/libregexvm
regexvminclude_HEADERS = lex.h regexvm_common.h regexvm_err.h regexvm.h \
						 stack.h stage1.h stage2.h vm.h vmcode.h \
						 string_builder.h

lib_LIBRARIES = libregexvm.a
libregexvm_a_SOURCES = stack.c lex.c stage1.c stage2.c vm.c regexvm.c vmcode.c \
					   randinput.c string_builder.c

AM_CFLAGS = -Wall -pedantic
