rxvmincludedir = $(includedir)/librxvm
rxvminclude_HEADERS = lex.h rxvm_common.h rxvm_err.h rxvm.h \
						 stack.h stage1.h stage2.h vm.h vmcode.h \
						 string_builder.h setop.h

lib_LIBRARIES = librxvm.a
librxvm_a_SOURCES = stack.c lex.c stage1.c stage2.c vm.c rxvm.c vmcode.c \
					   string_builder.c setop.c

AM_CFLAGS =-Wall -pedantic

if NOEXTRAS
AM_CFLAGS += -DNOEXTRAS
else
librxvm_a_SOURCES += bmh.c randinput.c lfix_to_str.c lfix.c
rxvminclude_HEADERS += bmh.h lfix_to_str.h lfix.h
endif
