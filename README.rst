librxvm: small and fast C library for regular expressions
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

#. GNU Make
#. GNU Autotools
#. A C compiler (GCC, Clang)

To install, do the usual stuff:
::

    ./autogen.sh
    ./configure
    make
    sudo make install

Usage
^^^^^

See sample code in the ``examples`` directory. The examples compile into simple
command-line programs, so you can look at the source to see how the library is
used, and then build them to try out some test expressions from your shell.

|


Example applications
--------------------

``rxvm_match``
~~~~~~~~~~~~~~
Accepts two arguments, a regular expression and an input
string. Prints a message indicating if the input string matches the expression
or not.

::

   $> rxvm_match

     Usage: rxvm_match <regex> <input>

   $> rxvm_match "[Rr]x(vm|VM){3,6}" "rxvm"

     No match.

   $> rxvm_match "[Rr]x(vm|VM){3,6}" "rxVMvmVM"

     Match!

``rxvm_search``
~~~~~~~~~~~~~~~
Accepts two arguments, a regular expression and an input
string. Prints any instances of the regular expression that occur inside the
input string.

::

   $> rxvm_search

     Usage: rxvm_search <regex> <input>

   $> rxvm_search "rx(vm)*" "------------rx---------"

     Found match: rx

   $> rxvm_search "rx(vm)*" "------rxvm-------rxvmvm----"

     Found match: rxvm
     Found match: rxvmvm

``rxvm_fsearch``
~~~~~~~~~~~~~~~~
Accepts two arguments, a regular expression and a filename.
Prints any instances of the regular expression that occur inside the file.

::

   $> rxvm_fsearch

     Usage: rxvm_fsearch <regex> <filename>

   $> echo "------rxvm-------rxvmvm----" > file.txt
   $> rxvm_fsearch "rx(vm)*" file.txt

     Found match: rxvm
     Found match: rxvmvm

``rxvm_gen``
~~~~~~~~~~~~
Accepts one argument, a regular expression. Generates a
pseudo-random string which matches the expression.

::

   $> rxvm_gen

     Usage: rxvm_gen <regex>

   $> rxvm_gen "([Rr]+(xv|XV)mm? ){2,}"

     rRrrRrrxvmm rxvmm rrRrrrRXVm Rrxvm rrRRrXVmm RXVmm

   $> rxvm_gen "([Rr]+(xv|XV)mm? ){2,}"

     Rxvm rrrxvmm RXVm RRxvmm

|

Regular Expression Syntax
-------------------------

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
    | **( )** | parenthesis group     | can contain any arbitrary expression, |
    |         |                       | and can be nested                     |
    +---------+-----------------------+---------------------------------------+
    | **[ ]** | character class       | can contain any number of literal     |
    |         |                       | characters (or escaped, i.e. to match |
    |         |                       | a literal ``[`` or ``]`` character) or|
    |         |                       | character ranges. Ranges are valid in |
    |         |                       | both directions, e.g. ``Z-A``         |
    |         |                       | describes the same set of characters  |
    |         |                       | as ``A-Z``                            |
    +---------+-----------------------+---------------------------------------+
    | **\\**  | escape                | used to remove special meaning from   |
    |         |                       | characters, e.g. to match  leteral    |
    |         |                       | ``*`` character                       |
    +---------+-----------------------+---------------------------------------+

|

Reference
---------

``rxvm_compile``
~~~~~~~~~~~~~~~~~~~

.. code:: c

   int rxvm_compile (rxvm_t *compiled, char *exp);

Compiles the regular expression ``exp``, and places the resulting VM
instructions into the ``rxvm_t`` type pointed to by ``compiled``.

|

**Return value**

* 0 if compilation succeeded
* negative number if an error occured (error codes defined in src/lex.h)

|

|

``rxvm_match``
~~~~~~~~~~~~~~~~~

.. code:: c

   int rxvm_match (rxvm_t *compiled, char *input, int flags);

Checks if the string ``input`` matches the compiled expression ``compiled``
exactly.

|

**Return value**

* 1 if the input matches the expression
* 0 if the input doesn't match the compiled expression
* RXVM_EMEM if memory allocation fails

|

|

``rxvm_search``
~~~~~~~~~~~~~~~~~~

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
* negative number if an error occured (error codes defined in src/lex.h)

|

|

``rxvm_free``
~~~~~~~~~~~~~~~~

.. code:: c

   void rxvm_free (rxvm_t *compiled);

Frees all dynamic memory associated with a compiled ``rxvm_t`` type. Always
call this function, before exiting, on any compiled ``rxvm_t`` types.

|

**Returns** nothing.

|

|

Non-essential features
----------------------

The following functions ``rxvm_fsearch``, ``rxvm_gen`` and ``rxvm_print``
are compiled in by default. However, if you don't need them and you want the
final library to be a bit smaller, you can exlude them by passing the
``--disable-extras`` flag to the ``configure`` script, i.e.

|

|

>  ``./configure --disable-extras``

|

|

``rxvm_fsearch``
~~~~~~~~~~~~~~~~~~~

.. code:: c

   int rxvm_fsearch (rxvm_t *compiled, FILE *fp, uint64_t *match_size, int flags);

Searches the file at ``fp`` (``fp`` must be initialised by the caller, e.g. via
``fopen``) for a pattern that matches the compiled regular expresssion
``compiled``, from the current file position until EOF. If a match is found,
the file pointer ``fp`` is re-positioned to the first character of the match,
and ``match_size`` is populated with a positive integer representing the match
size (number of characters). If no match is found, then ``match_end`` is set to
0, and ``fp`` remains positioned at EOF.

**Return value**

* 1 if a match is found
* 0 if no match is found
* negative number if an error occured (error codes defined in src/lex.h)

|

|

``rxvm_gen``
~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~

.. code:: c

   void rxvm_print (rxvm_t *compiled)

Prints a compiled expression in a human-readable format.

**Returns** nothing.

|

|

Flags
-----

``rxvm_match`` and ``rxvm_search`` take a ``flags`` parameter. You can use
the masks below to set bit-flags which will change the behaviour of these
functions (combine multiple flags by bitwise OR-ing them together):

|

``RXVM_ICASE``
~~~~~~~~~~~~~~~~~

case insensitive: ignore case when matching alphabet characters. Matching is
case-sensitive by default.

``RXVM_NONGREEDY``
~~~~~~~~~~~~~~~~~~~~~

non-greedy matching: by default, the operators ``+``, ``*``, and ``?`` will
match as many characters as possible, e.g. running ``rxvm_search`` with
the expression ``<.*>`` against the input string ``<tag>name<tag>`` will match
the entire string. With this flag set, it will match only ``<tag>``.

``RXVM_MULTILINE``
~~~~~~~~~~~~~~~~~~~~~

Multiline: By default, ``^`` matches immediately following the start of input,
and ``$`` matches immediately preceding the end of input or the newline before
the end of input. With this flag set, ``^`` will also match immediately
following each newline character, and ``$`` will also match immediately
preceding each newline character. This flag is ignored and automatically
enabled when ``rxvm_match`` is used; since ``rxvm_match`` effectively
requires a matching string to be anchored at both the start and end of input,
then ``^`` and ``$`` are only useful if they can also act as line anchors.

|

Building your own code with librxvm
--------------------------------------

To link your own code with librxvm, compile with
::

    -I/usr/local/include/librxvm

and link with
::

    -lrxvm

for example, to build the example applications manually, you would do
::

    cd examples
    gcc rxvm_search.c -o rxvm_search -I/usr/local/include/librxvm -lrxvm
    gcc rxvm_match.c -o rxvm_match -I/usr/local/include/librxvm -lrxvm
