libregexvm: small and fast C library for regular expressions
============================================================

This implementation is inspired by
`Russ Cox's article <https://swtch.com/~rsc/regexp/regexp2.html>`_ on the
*virtual machine* approach to implementing regular expressions.

Some benefits to this approach:

#. No backtracking over the input string.
#. Matches complete in **O(n * m)** worst-case time, where **n** is the
   input string size and **m** is the expression size (linear time, given that
   **m** is fixed for a given expression).
#. The amount of dynamic memory used depends only on the expression size. It is
   independent of the input string size.

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

See sample code in the **examples** directory. The examples compile into simple
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


Supported metacharacters
------------------------


    +---------+-----------------------+---------------------------------------+
    |*Symbol* | *Name*                | *Description*                         |
    +=========+=======================+=======================================+
    | **+**   | one or more           | matches one or more of the preceding  |
    |         |                       | expression                            |
    +---------+-----------------------+---------------------------------------+
    | **\***  | zero or more          | matches zero or more of the preceding |
    |         |                       | expression                            |
    +---------+-----------------------+---------------------------------------+
    | **?**   | zero or one           | matches zero or one of the preceding  |
    |         |                       | expression                            |
    +---------+-----------------------+---------------------------------------+
    | **|**   | alternation           | allows either the preceding or the    |
    |         |                       | following expression to match         |
    +---------+-----------------------+---------------------------------------+
    | **.**   |  any                  | matches any character                 |
    +---------+-----------------------+---------------------------------------+
    | **( )** | parenthesis groups    | can contain any arbitrary expression, |
    |         |                       | and can be nested                     |
    +---------+-----------------------+---------------------------------------+
    | **[ ]** | character class       | can contain any number of literal     |
    |         |                       | characters (or escaped, i.e. to match |
    |         |                       | a literal [ or ] character) or        |
    |         |                       | character ranges. Ranges are valid in |
    |         |                       | both directions, e.g. Z-A describes   |
    |         |                       | the same set of characters as A-Z     |
    +---------+-----------------------+---------------------------------------+
    | **\\**  | escape                | used to remove special meaning from   |
    |         |                       | characters, e.g. to match a literal   |
    |         |                       | * character                           |
    +---------+-----------------------+---------------------------------------+


Building your own code with libregexvm
--------------------------------------

To link your own code with libregexvm, compile with
::

    -I/usr/local/include/libregexvm

and link with
::

    -lregexvm

for example, to build the example applications manually, you would do
::

    cd examples
    gcc regexvm_iter.c -o regexvm_iter -I/usr/local/include/libregexvm -lregexvm
    gcc regexvm_match.c -o regexvm_match -I/usr/local/include/libregexvm -lregexvm
