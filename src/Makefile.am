rxvmincludedir = $(includedir)/librxvm
rxvminclude_HEADERS = lex.h rxvm_common.h rxvm_err.h rxvm.h \
						 stack.h stage1.h stage2.h vm.h vmcode.h \
						 string_builder.h setop.h

lib_LIBRARIES = librxvm.a
librxvm_a_SOURCES = stack.c lex.c stage1.c stage2.c vm.c rxvm.c vmcode.c \
					   randinput.c string_builder.c setop.c

AM_CFLAGS =-W -pedantic
