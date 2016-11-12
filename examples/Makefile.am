bin_PROGRAMS = rxvm_match rxvm_search
LDADD = -lrxvm -L../src
AM_CFLAGS = -I../src
AM_DEFAULT_SOURCE_EXT = .c

if !NOEXTRAS
bin_PROGRAMS += rxvm_fsearch rxvm_gen
else
AM_CFLAGS += -DNOEXTRAS
endif
