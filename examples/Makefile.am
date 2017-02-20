bin_PROGRAMS = rxvm_match rxvm_search
LDADD = -lrxvm -L../librxvm
AM_CFLAGS = -I..
AM_DEFAULT_SOURCE_EXT = .c

if NOEXTRAS
AM_CFLAGS += -DNOEXTRAS
else
bin_PROGRAMS += rxvm_fsearch rxvm_gen rxvm_print
endif
