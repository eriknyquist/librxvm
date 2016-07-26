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

See sample code in the ``examples`` directory. The examples compile into simple
command-line programs, so you can look at the source to see how the library is
used, and then build them to try out some test expressions from your shell.

|

Building the examples
---------------------

After running the main Makefile from the **Installation** section, the examples
can be built like so:
::

    cd examples
    make

Using the examples
------------------
::

   $> ./regexvm_search

     Usage: ./regexvm_search <regex> <input>

   $>./regexvm_search "regex(vm)*" "UtrUygHIuregexvmvmvmllTRDrHIOIP"

     Match!
     Found matching substring:
     regexvmvmvm

   $> ./regexvm_search "regex(vm)*" "UtrUygHIuregexllTRDrHIOIP"

     Match!
     Found matching substring:
     regex

   $> ./regexvm_search "regex(vm)*" "UtrUygHIuregellTRDrHIOIP"

     No match.

   $>

|

Regular Expression Syntax
-------------------------

A regular expression consists of ordinary characters and special characters.
An ordinary character matches itself exactly (e.g. the expression ``abc``
matches only the input string ``abc``).

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
    |         |                       | string. If the REGEXVM_MULTILINE flag |
    |         |                       | is set, then it also matches          |
    |         |                       | immediately following each newline    |
    |         |                       | character                             |
    +---------+-----------------------+---------------------------------------+
    | **$**   | end anchor            | by default, matches immediately       |
    |         |                       | preceding the end of the input string |
    |         |                       | or newline character at the end of the|
    |         |                       | input string. If the REGEXVM_MULTILINE|
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

``regexvm_compile``
~~~~~~~~~~~~~~~~~~~

.. code:: c

   int regexvm_compile (regexvm_t *compiled, char *exp);

Compiles the regular expression ``exp``, and places the resulting VM
instructions into the ``regexvm_t`` type pointed to by ``compiled``.

|

**Return value**

* 0 if compilation succeeded
* negative number if an error occured (error codes defined in src/lex.h)

|

|

``regexvm_match``
~~~~~~~~~~~~~~~~~

.. code:: c

   int regexvm_match (regexvm_t *compiled, char *input, int flags);

Check if the string ``input`` matches the compiled expression ``compiled``
exactly.

|

**Return value**

* 1 if the input matches the expression
* 0 if the input doesn't match the compiled expression
* RVM_EMEM if memory allocation fails

|

|

``regexvm_search``
~~~~~~~~~~~~~~~~~~

.. code:: c

   int regexvm_search (regexvm_t *compiled, char *input, char **start, char **end, int flags);

Searches the string starting at ``input`` for a pattern that matches the
compiled regular expresssion ``compiled``, until a match is found or until the
string's null termination character is reached. When a match is found,
the pointers pointed to by ``start`` and ``end`` are populated with the
locations within the input string where the matching portion starts and ends,
respectively. If no match is found, then both ``start`` and ``end`` will be set
to ``NULL``.

|

**Return value**

* 1 if a match is found
* 0 if no match is found
* negative number if an error occured (error codes defined in src/lex.h)

|

|

``regexvm_fsearch``
~~~~~~~~~~~~~~~~~~~

.. code:: c

   int regexvm_fsearch (regexvm_t *compiled, FILE *fp, uint64_t *match_size, int flags);

Searches the file at ``fp`` (``fp`` must be initialised by the caller, e.g. via
``fopen``) for a pattern that matches the compiled regular expresssion
``compiled``, from the current file position until EOF. If a match is found,
the file pointer ``fp`` is re-positioned to the first character of the match,
and ``match_size`` is populated with a positive integer representing the match
size (number of characters). If no match is found, then ``match_end`` will be
set to 0, and ``fp`` will remain positioned at EOF.

**Return value**

* 1 if a match is found
* 0 if no match is found
* negative number if an error occured (error codes defined in src/lex.h)

|

|

``regexvm_gen``
~~~~~~~~~~~~~~~

.. code:: c

   char *regexvm_gen (regexvm_t *compiled, rxvm_gencfg_t *cfg);

Generates a string of random characters that matches the compiled expression
``compiled`` (``compiled`` must be initialised by the caller first, e.g. via
``regexvm_compile``).

The ``rxvm_genfg_t`` type provides some control over the randomness:

.. code:: c

   struct rxvm_gencfg {
       uint8_t generosity;
       uint8_t whitespace;
   };

* ``generosity``: 0-100, representing the probability out of 100 that a ``+`` or
  ``*`` operator will match again ("greedyness" in reverse). Higher means
  more repeat matches.
* ``whitespace``: 0-100, representing the probability that a whitespace
  character will be used instead of a visible character, when the expression
  allows it (e.g. when the expression contains a "." metacharacter). Higher
  means more whitespace.

If a null pointer is passed instead of a valid pointer to a ``rxvm_genfg_t``
type, then default probability values will be used.

**Return value**

A pointer to a heap allocation that contains a null-terminated random
matching string. If memory allocation fails, a null pointer is returned.

|

|


``regexvm_free``
~~~~~~~~~~~~~~~~

.. code:: c

   void regexvm_free (regexvm_t *compiled);

Frees all dynamic memory associated with a compiled ``regexvm_t`` type. Always
call this function, before exiting, on any compiled ``regexvm_t`` types.

|

**Returns** nothing.

|

|

``regexvm_print``
~~~~~~~~~~~~~~~~~

.. code:: c

   void regexvm_print (regexvm_t *compiled)

Prints a compiled expression in a human-readable format.

**Returns** nothing.

|

Flags
-----

``regexvm_match`` and ``regexvm_search`` take a ``flags`` parameter. You can use
the masks below to set bit-flags which will change the behaviour of these
functions (combine multiple flags by bitwise OR-ing them together):

|

``REGEXVM_ICASE``
~~~~~~~~~~~~~~~~~

case insensitive: ignore case when matching alphabet characters. Matching is
case-sensitive by default.

``REGEXVM_NONGREEDY``
~~~~~~~~~~~~~~~~~~~~~

non-greedy matching: by default, the operators ``+``, ``*``, and ``?`` will
match as many characters as possible, e.g. running ``regexvm_search`` with
the expression ``<.*>`` against the input string ``<tag>name<tag>`` will match
the entire string. With this flag set, it will match only ``<tag>``.

``REGEXVM_MULTILINE``
~~~~~~~~~~~~~~~~~~~~~

Multiline: By default, ``^`` matches immediately following the start of input,
and ``$`` matches immediately preceding the end of input or the newline before
the end of input. With this flag set, ``^`` will also match immediately
following each newline character, and ``$`` will also match immediately
preceding each newline character. This flag is ignored and automatically
enabled when ``regexvm_match`` is used; since ``regexvm_match`` effectively
requires a matching string to be anchored at both the start and end of input,
then ``^`` and ``$`` are only useful if they can also act as line anchors.

|

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
    gcc regexvm_search.c -o regexvm_search -I/usr/local/include/libregexvm -lregexvm
    gcc regexvm_match.c -o regexvm_match -I/usr/local/include/libregexvm -lregexvm
