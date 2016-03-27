AC_PREREQ([2.69])
AC_INIT(regexvm, 0.5, eknyquist@gmail.com)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/stack.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE
AC_REQUIRE_AUX_FILE([tap-driver.sh])

CFLAGS="-Wall -O3"
AC_PROG_CC
AC_PROG_AWK
AC_PROG_INSTALL

AC_CHECK_HEADERS([stdint.h stdlib.h string.h])

AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_PROG_RANLIB

AM_PROG_AR

AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AC_OUTPUT(Makefile src/Makefile tests/src/Makefile examples/Makefile)
