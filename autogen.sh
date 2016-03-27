#! /bin/sh

[ -d build-aux ] || mkdir build-aux
autoreconf --install
aclocal && automake --add-missing && autoconf
