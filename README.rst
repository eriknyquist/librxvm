libregexvm: small and fast C library for regular expressions
============================================================

Installation
^^^^^^^^^^^^

**Dependencies:**

#. GNU Autotools
#. A C compiler

To install, do the usual stuff:
::

    ./autogen.sh
    ./configure
    make
    sudo make install

Usage
^^^^^

See sample code in the *examples* directory. The examples compile into simple
command-line programs, so you can look at the source to see how the library is
used, and then build them to try out some test expressions from your shell.

|

Building the examples
---------------------

**NOTE: You must install libregexvm before building the examples**
::

    cd examples
    make

Using the examples
------------------
::

   $> ./regexvm_iter

     Usage: ./regexvm_iter <regex> <input>

   $>./regexvm_iter "regex(vm)*" "UtrUygHIuregexvmvmvmllTRDrHIOIP"

     Match!
     Found matching substring:
     regexvmvmvm

   $> ./regexvm_iter "regex(vm)*" "UtrUygHIuregexllTRDrHIOIP"

     Match!
     Found matching substring:
     regex

   $> ./regexvm_iter "regex(vm)*" "UtrUygHIuregellTRDrHIOIP"

     No match.

   $>

To link your own code with libregexvm, compile with
::

    -I/usr/local/include/libregexvm

and link with
::

    -L/usr/local/lib -lregexvm
