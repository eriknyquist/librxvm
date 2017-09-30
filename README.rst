librxvm: non-backtracking NFA-based C library for regular expressions
=====================================================================

.. image:: https://travis-ci.org/eriknyquist/librxvm.svg?branch=master
    :target: https://travis-ci.org/eriknyquist/librxvm

.. image:: https://ci.appveyor.com/api/projects/status/v7mj5v1xjg8rqdr5?svg=true
    :target: https://ci.appveyor.com/project/eriknyquist/librxvm
|

.. contents:: Table of Contents

Introduction
------------

Short one
^^^^^^^^^

``librxvm`` is a C library for matching regular expressions on large sets of
data.

Long one
^^^^^^^^

``librxvm`` is a **R**\ egular e\ **X**\ pression **V**\ irtual **M**\ achine.
It compiles a regular expression into an NFA representation consisting of a
sequence of primitive opcodes for a "`virtual machine <https://swtch.com/~rsc/regexp/regexp2.html>`_"
(the "machine" here is theoretical, rather than physical hardware, and is
implemented as  a simple bytecode interpreter). The virtual machine can then
execute the NFA representation, or "program" against some input text to
determine whether the input text matches the regular expression.

In addition to the usual string matching & searching functions, librxvm also
provides a function (``rxvm_fsearch``) that takes a FILE pointer for large sets
of input data. ``rxvm_fsearch`` uses the Boyer-Moore-Horspool algorithm to
achieve **extremely high** throughput for regular expression searches on any
data that can be accessed through a standard ``FILE`` pointer. You can try it
out with the ``rxvm_fsearch`` example application.

Speed test: using the ``rxvm_fsearch`` function to search a 1GB plain-text file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

    $> ls -lh file
    -rw-r--r-- 1 enyquist enyquist 954M Jul 23 21:55 file

    $> time examples/rxvm_fsearch "eriknyquist" file

    eriknyquistB6dvGZ7wxvd9pqXnep5wABDee7UqaNibTUch/LUAcAjqRPAnQBpMjdG3w7R4wR+1082/ctpmmWk

    real    0m0.528s
    user    0m0.444s
    sys 0m0.084s

    $>

528 milliseconds-- pretty fast. In fact, if we search for longer strings, we can
go even faster, because of how the Boyer-Moore string searching algorithm works

::

    $> time examples/rxvm_fsearch "this is a long string" file

    real    0m0.424s
    user    0m0.300s
    sys 0m0.120s

    $> time examples/rxvm_fsearch "this is a very very very very veeeeeeeeeerrrrrrrrrryyyyyyyyyyyyy long string" file

    real    0m0.197s
    user    0m0.068s
    sys 0m0.128s

Under 200 milliseconds to search a 1GB file, when the search pattern contains a
fixed string of ~80 characters or more. That's fast!

What's missing from ``librxvm``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Currently, ``librxvm`` only works with plain ol' ASCII.
* It's not POSIX compliant, or anything compliant as far as I know.
* Probably a lot of things.

``librxvm`` Regular Expression Syntax
-------------------------------------

A regular expression consists of ordinary characters and special characters.
An ordinary character matches itself exactly (e.g. the expression ``abc``
matches only the input string ``abc``).

Full grammar rules can be seen `here <https://github.com/eriknyquist/regexvm/blob/master/tests/grammar.txt>`_.

|

A description of the available special characters follows.


    +---------+-----------------------+---------------------------------------+
    |*Symbol* | *Name*                | *Description*                         |
    +=========+=======================+=======================================+
    | **+**   | one or more           | matches one or more of the preceding  |
    |         |                       | character or parenthesis group, e.g.  |
    |         |                       | the expression ``ab+`` matches the    |
    |         |                       | input ``ab``, ``abb``, but not ``a``  |
    +---------+-----------------------+---------------------------------------+
    | **\***  | zero or more          | matches zero or more of the preceding |
    |         |                       | character or parenthesis group, e.g.  |
    |         |                       | the expression ``ab*`` matches the    |
    |         |                       | input ``a``, ``ab`` and ``abb``       |
    +---------+-----------------------+---------------------------------------+
    | **?**   | zero or one           | matches zero or one of the preceding  |
    |         |                       | character or parenthesis group, e.g   |
    |         |                       | the expression ``ab?`` matches only   |
    |         |                       | ``a`` or ``ab``                       |
    +---------+-----------------------+---------------------------------------+
    | **{n}** | repetition            | matches **n** repetitions of the      |
    |         |                       | preceding character or parenthesis    |
    |         |                       | group.                                |
    +---------+-----------------------+---------------------------------------+
    |**{n,m}**| repetition (range)    | matches **n** to **m** repetitions of |
    |         |                       | the preceding character or parenthesis|
    |         |                       | group.                                |
    +---------+-----------------------+---------------------------------------+
    | **{,m}**| repetition (less)     | matches **m** or fewer repetitions of |
    |         |                       | the preceding character or parenthesis|
    |         |                       | group                                 |
    +---------+-----------------------+---------------------------------------+
    | **{n,}**| repetition (more)     | matches **n** or more repetitions of  |
    |         |                       | the preceding character or parenthesis|
    |         |                       | group                                 |
    +---------+-----------------------+---------------------------------------+
    | **|**   | alternation           | allows either the preceding or the    |
    |         |                       | following expression to match, e.g.   |
    |         |                       | the expression ``(c|h)at`` matches    |
    |         |                       | ``cat`` and ``hat``                   |
    +---------+-----------------------+---------------------------------------+
    | **.**   | any                   | matches any character                 |
    +---------+-----------------------+---------------------------------------+
    | **^**   | start anchor          | by default, matches immediately       |
    |         |                       | following the beginning of the input  |
    |         |                       | string. If the RXVM_MULTILINE flag    |
    |         |                       | is set, then it also matches          |
    |         |                       | immediately following each newline    |
    |         |                       | character                             |
    +---------+-----------------------+---------------------------------------+
    | **$**   | end anchor            | by default, matches immediately       |
    |         |                       | preceding the end of the input string |
    |         |                       | or newline character at the end of the|
    |         |                       | input string. If the RXVM_MULTILINE   |
    |         |                       | flag is set, then it also matches     |
    |         |                       | immediately preceding each newline    |
    |         |                       | character                             |
    +---------+-----------------------+---------------------------------------+
    | **( )** | parenthesis group     | Groups together individual characters |
    |         |                       | or subexpressions, e.g. ``a(bc)+``    |
    |         |                       | matches ``abcbc`` or ``abcbcbcbc``,   |
    |         |                       | but not ``a``. Parenthesis groups can |
    |         |                       | contain any expression, and can be    |
    |         |                       | nested.                               |
    +---------+-----------------------+---------------------------------------+
    | **[ ]** | character class       | matches a single character inside     |
    |         |                       | the brackets. Characters can be       |
    |         |                       | escaped, (e.g. to match a literal     |
    |         |                       | ``"["`` or ``"]"`` character), or part|
    |         |                       | of a range. Ranges are valid in both  |
    |         |                       | valid in both directions, e.g.        |
    |         |                       | ``Z-A`` describes the same set of     |
    |         |                       | characters as ``A-Z``                 |
    +---------+-----------------------+---------------------------------------+
    |**[^ ]** | negated character     | matches a single character *not*      |
    |         | class                 | inside the brackets. Otherwise, the   |
    |         |                       | same character class rules apply      |
    +---------+-----------------------+---------------------------------------+
    | **\\**  | escape                | used to remove special meaning from   |
    |         |                       | characters, e.g. to match a literal   |
    |         |                       | ``*`` character                       |
    +---------+-----------------------+---------------------------------------+

|

Installation
------------

Installing librxvm as a python module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Just use the included ``setup.py`` file to compile & install ``librvm`` as a
python module:

::

    python setup.py build
    python setup.py install

`Full documentation for the librxvm python API can be found
here <https://github.com/eriknyquist/regexvm/blob/master/pyrxvm/README.rst>`_

Installing librxvm as a C library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Dependencies:**

#. GNU Make
#. GNU Autotools
#. A C compiler (GCC, Clang)
#. Some kind of libc (requires ``stdio.h``, ``stdlib.h``, ``stdint.h`` and
   ``string.h``)

To install, do the usual stuff:
::

    ./autogen.sh
    ./configure
    make
    sudo make install

The resulting static library ``librxvm.a`` will be installed in
``/usr/local/lib`` by default. With the default configuration (i.e. everything
turned on), the compiled library is about 54K on my 64-bit system. If you want
to shave off ~40% of this size (around 20K in my case), you can configure with
the ``--disable-extras`` flag (see `Reference: optional features`_).

Building C code with librxvm
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Once librxvm is installed, you can use it by adding
``#include <librxvm/rxvm.h>`` to your program, and then passing ``-lrxvm`` when
linking. For example:
::

    gcc my_rxvm_program.c -lrxvm


Example applications
--------------------

See sample code in the ``examples`` directory. The examples are simple, and
compile into easy-to-use command-line programs. They are automatically built by
the top-level Makefile when you run ``make`` to build ``librxvm``.

``rxvm_match``
^^^^^^^^^^^^^^
Accepts two arguments, a regular expression and an input
string. Prints a message indicating whether the input string matches the
expression or not.

::

   $> examples/rxvm_match

     Usage: rxvm_match <regex> <input>

   $> examples/rxvm_match "[Rr]x(vm|VM){3,6}" "rxvm"

     No match.

   $> examples/rxvm_match "[Rr]x(vm|VM){3,6}" "rxVMvmVM"

     Match!

``rxvm_search``
^^^^^^^^^^^^^^^
Accepts two arguments, a regular expression and an input
string. Prints any instances of the regular expression that occur inside the
input string.

::

   $> examples/rxvm_search

     Usage: rxvm_search <regex> <input>

   $> examples/rxvm_search "rx(vm)*" "------------rx---------"

     Found match: rx

   $> examples/rxvm_search "rx(vm)*" "------rxvm-------rxvmvm----"

     Found match: rxvm
     Found match: rxvmvm

``rxvm_fsearch``
^^^^^^^^^^^^^^^^

Accepts two arguments, a regular expression and a filename.
Prints any instances of the regular expression that occur inside the file.

::

   $> examples/rxvm_fsearch

     Usage: rxvm_fsearch <regex> <filename>

   $> echo "------rxvm-------rxvmvm----" > file.txt
   $> examples/rxvm_fsearch "rx(vm)*" file.txt

     Found match: rxvm
     Found match: rxvmvm

``rxvm_gen``
^^^^^^^^^^^^
Accepts one argument, a regular expression. Generates a
pseudo-random string which matches the expression.

::

   $> examples/rxvm_gen

     Usage: rxvm_gen <regex>

   $> examples/rxvm_gen "([Rr]+(xv|XV)mm? ){2,}"

     rRrrRrrxvmm rxvmm rrRrrrRXVm Rrxvm rrRRrXVmm RXVmm

   $> examples/rxvm_gen "([Rr]+(xv|XV)mm? ){2,}"

     Rxvm rrrxvmm RXVm RRxvmm

|

Reference: core features
------------------------

``rxvm_compile``
^^^^^^^^^^^^^^^^

.. code:: c

   int rxvm_compile (rxvm_t *compiled, char *exp);

Compiles the regular expression ``exp``, and places the resulting VM
instructions into the ``rxvm_t`` type pointed to by ``compiled``.

|

**Return value**

* 0 if compilation succeeded
* negative number if an error occured (See `rxvm_compile error codes`_) and
  `General error codes`_)

|

|

``rxvm_match``
^^^^^^^^^^^^^^

.. code:: c

   int rxvm_match (rxvm_t *compiled, char *input, int flags);

Checks if the string ``input`` matches the compiled expression ``compiled``
exactly.

|

**Return value**

* 1 if the input matches the expression
* 0 if the input doesn't match the compiled expression
* negative number if an error occured (See `General error codes`_)

|

|

``rxvm_search``
^^^^^^^^^^^^^^^

.. code:: c

   int rxvm_search (rxvm_t *compiled, char *input, char **start, char **end, int flags);

Searches the string starting at ``input`` for a pattern that matches the
compiled regular expresssion ``compiled``, until a match is found or until the
string's null termination character is reached. When a match is found,
the pointers pointed to by ``start`` and ``end`` are pointed at the first and
last characters of the matching substring. If no match is found, then both
``start`` and ``end`` are set to ``NULL``.

|

**Return value**

* 1 if a match is found
* 0 if no match is found
* negative number if an error occured (See `General error codes`_)

|

|

``rxvm_free``
^^^^^^^^^^^^^

.. code:: c

   void rxvm_free (rxvm_t *compiled);

Frees all dynamic memory associated with a compiled ``rxvm_t`` type. Always
call this function, before exiting, on any compiled ``rxvm_t`` types.

|

**Returns** nothing.

|

|

Reference: optional features
----------------------------

The following functions ``rxvm_fsearch``, ``rxvm_gen`` and ``rxvm_print``
are compiled in by default. However, if you don't need them and you want the
final library to be a bit smaller, you can exlude them by passing the
``--disable-extras`` flag to the ``configure`` script, e.g.

|

|

>  ``./configure --disable-extras``

|

|

``rxvm_fsearch``
^^^^^^^^^^^^^^^^

.. code:: c

   int rxvm_fsearch (rxvm_t *compiled, FILE *fp, uint64_t *match_size, int flags);

Searches the file at ``fp`` (``fp`` must be initialised by the caller, e.g. via
``fopen``) for a pattern that matches the compiled regular expresssion
``compiled``, from the current file position until EOF. If a match is found,
the file pointer ``fp`` is re-positioned to the first character of the match,
and ``match_size`` is populated with a positive integer representing the match
size (number of characters). If no match is found, then ``match_end`` is set to
0, and ``fp`` remains positioned at EOF.

This function uses an implementation of the Boyer-Moore-Horspool (BMH) algorithm
to search the file for a pattern, and can be extremely fast. Because the
BMH algorithm only works with fixed strings, this function uses a special
heuristic to identify subtrings of fixed literal characters in your expression,
and uses the fast BMH algorithm to search for these smaller substrings. If one
is found, the virtual machine is invoked (needed to match a regular expression,
but slower).

This means the type of expression you write can significantly affect the speed
of the ``rxvm_search`` function. Specifically, **longer** strings means
**faster** matching.


**Return value**

* 1 if a match is found
* 0 if no match is found
* negative number if an error occured (See `rxvm_fsearch error codes`_ and
  `General error codes`_)

|

|

``rxvm_gen``
^^^^^^^^^^^^

.. code:: c

   char *rxvm_gen (rxvm_t *compiled, rxvm_gencfg_t *cfg);

Generates a string of random characters that matches the compiled expression
``compiled`` (``compiled`` must be initialised by the caller first, e.g. via
``rxvm_compile``).

The ``rxvm_gencfg_t`` type provides some control over the randomness:

.. code:: c

   struct rxvm_gencfg {
       uint8_t generosity;
       uint8_t whitespace;
       uint64_t limit;

       uint64_t len;
   };

* ``generosity``: This value is expected to be between 0-100, and represents the
  probability out of 100 that a ``+`` or ``*`` operator will match again
  ("greedyness" in reverse). Higher means more repeat matches.
* ``whitespace``: This value is expected to be between 0-100, and represents the
  probability that a whitespace character will be used instead of a visible
  character, when the expression allows it (e.g. when the expression contains a
  "." metacharacter). Higher means more whitespace.
* ``limit``: This value represents the generated input string size at which the
  generation process should stop. This is not hard limit on the size of the
  generated string; when the generated string reaches a size of ``limit``, then
  ``generosity`` is effectively set to 0, and generation will stop at the
  earliest possible opportunity, while also ensuring that the generated string
  matches the pattern ``compiled``.
* ``len``: If ``rxvm_gen`` returns a valid (non-null) pointer, then ``len`` will
  contain the number of characters in the generated string (excluding the
  terminating null-character).

If a null pointer is passed instead of a valid pointer to a ``rxvm_gencfg_t``
type, then default values are used.

**Return value**

A pointer to a heap allocation that contains a null-terminated random
matching string. If memory allocation fails, a null pointer is returned.

|

|


``rxvm_print``
^^^^^^^^^^^^^^

.. code:: c

   void rxvm_print (rxvm_t *compiled)

Prints a compiled expression in a human-readable format.

**Returns** nothing.

|

|

Constants
---------

Flags
^^^^^

``rxvm_match`` and ``rxvm_search`` take a ``flags`` parameter. You can use
the masks below to set bit-flags which will change the behaviour of these
functions (combine multiple flags by bitwise OR-ing them together):

|

``RXVM_ICASE``
##############

case insensitive: ignore case when matching alphabet characters. Matching is
case-sensitive by default.

``RXVM_NONGREEDY``
##################

non-greedy matching: by default, the operators ``+``, ``*``, and ``?`` will
match as many characters as possible, e.g. running ``rxvm_search`` with
the expression ``<.*>`` against the input string ``<tag>name<tag>`` will match
the entire string. With this flag set, it will match only ``<tag>``.

``RXVM_MULTILINE``
##################

Multiline: By default, ``^`` matches immediately following the start of input,
and ``$`` matches immediately preceding the end of input or the newline before
the end of input. With this flag set, ``^`` will also match immediately
following each newline character, and ``$`` will also match immediately
preceding each newline character. This flag is ignored and automatically
enabled when ``rxvm_match`` is used; since ``rxvm_match`` effectively
requires a matching string to be anchored at both the start and end of input,
then ``^`` and ``$`` are only useful if they can also act as line anchors.

General error codes
^^^^^^^^^^^^^^^^^^^

The following error codes are returned by all ``librxvm`` functions

|

``RXVM_EMEM``
#############

Indicates that memory allocation failed.

|

``RXVM_EPARAM``
###############

Indicates that an invalid parameter (e.g. a ``NULL`` pointer) was passed to a
``librxvm`` library function.

|

``rxvm_compile`` error codes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following error codes are returned only by the ``rxvm_compile`` function

|

``RXVM_BADOP``
##############

Indicates that an operator (``*``, ``+``, ``?``, ``{}``) was used incorrectly
in the input expression, i.e. without a preceding literal character.

|

Example expressions: ``ab++``, ``{5}``.

|

``RXVM_BADCLASS``
#################

Indicates that an unexpected (and unescaped) character class closing character
(``]``) was encountered in the input expression.

|

Example expressions: ``xy]``, ``[a-f]]``

|

``RXVM_BADREP``
###############

Indicates that an unexpected (and unescaped) repetition closing character
(``}``) was encountered in the input expression.

|

Example expressions: ``a}``, ``bb{4,}}``

|

``RXVM_BADPAREN``
#################

Indicates that an unexpected (and unescaped) closing parenthesis character
(``)``) was encountered in the input expression.

|

Example expressions: ``qy)``, ``q*(ab))``

|

``RXVM_EPAREN``
###############

Indicates that an unterminated parenthesis group (``()``) was encountered in
the input expression.

|

Example expressions: ``d+(ab``, ``((ab)``

|

``RXVM_ECLASS``
###############

Indicates that an unterminated character class (``[]``) was encountered in
the input expression.

|

Example expressions: ``[A-Z``, ``[[A-Z]``

|

``RXVM_EREP``
#############

Indicates that an unterminated repetition (``{}``) was encountered in
the input expression.

|

Example expressions: ``ab{5``, ``((ab)``

|

``RXVM_ERANGE``
###############

Indicates that an incomplete character range inside a character class was
encountered in the input expression.

|

Example expressions: ``[A-]``, ``[-z]``

|

``RXVM_ECOMMA``
###############

Indicates that an invalid extra comma inside a repetition was encountered in
the input expression.

|

Example expressions: ``ab{5,,}``, ``x{6,7,8}``

|

``RXVM_EDIGIT``
###############

Indicates that an invalid character (i.e. not a digit or a comma) inside a
repetition was encountered in the input expression.

|

Example expressions: ``ab{3,y}``, ``b{8.9}``

|

``RXVM_MREP``
#############

Indicates that an empty repetition (``{}``) was encountered in
the input expression.

|

Example expressions: ``ab{}``, ``ab{,}``

|

``RXVM_ETRAIL``
###############

Indicates that a trailing escape character (``\\``) was encountered in
the input expression.

|

Example expressions: ``ab\\``, ``\\*\\``

|

``RXVM_EINVAL``
###############

Indicates that an invalid symbol (any character outside the supported
character set) was encountered in the input expression.

|

``rxvm_fsearch`` error codes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following error codes are returned only by the ``rxvm_fsearch`` function

|

``RXVM_IOERR``
##############

Indicates that an error occured while attempting to read from the passed
``FILE`` pointer

|

Test Suite
----------

To run the tests, use the ``check`` target in the main Makefile
::

    make check

You can also run the tests through Valgrind (if installed) to check for memory
leaks or other issues in ``librxvm``, using the separate Makefile provided
specifically for this purpose, ``memcheck.mk``

NOTE: Running the tests through Valgrind can take a very long time to complete

::

    make -f memcheck.mk
