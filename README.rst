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

|

Supported metacharacters
------------------------


    +---------+-----------------------+---------------------------------------+
    |*Symbol* | *Name*                | *Description*                         |
    +=========+=======================+=======================================+
    | **+**   | one or more           | matches one or more of the preceding  |
    |         |                       | character, e.g. the expression ``ab+``|
    |         |                       | matches the input ``ab``, ``abb``, but|
    |         |                       | not ``a``                             |
    +---------+-----------------------+---------------------------------------+
    | **\***  | zero or more          | matches zero or more of the preceding |
    |         |                       | character, e.g. the expression ``ab*``|
    |         |                       | matches the input ``a``, ``ab`` and   |
    |         |                       | ``abb``                               |
    +---------+-----------------------+---------------------------------------+
    | **?**   | zero or one           | matches zero or one of the preceding  |
    |         |                       | character, e.g the expression ``ab?`` |
    |         |                       | matches only ``a`` or ``ab``          |
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
    |         |                       | characters, e.g. to match a literal   |
    |         |                       | ``*`` character                       |
    +---------+-----------------------+---------------------------------------+

|

Reference
---------

``regexvm_compile``
~~~~~~~~~~~~~~~~~~~

.. code:: c

   int regexvm_compile (regexvm_t *compiled, char *exp)

Compiles the regular expression ``exp``, and places the resulting VM
instructions into the ``regexvm_t`` type pointed to by ``compiled``.

|

**Returns** 0
on success, otherwise one of the error codes defined (and commented) in lex.h.

|

|

``regexvm_match``
~~~~~~~~~~~~~~~~~

.. code:: c

   int regexvm_match (regexvm_t *compiled, char *input, int flags)

Performs a one-shot execution of the VM, using the instructions in the
``regexvm_t`` type pointed to by ``compiled`` (which must have already been
populated by ``regexvm_compile()``) and the input string ``input``.

|

**Returns** 1
if the input string matches the expression exactly, and 0 if the input string
doesn't match. The only error this function can return is RVM_EMEM, which it
will do if it fails to allocate memory.

|

|

``regexvm_iter``
~~~~~~~~~~~~~~~~

.. code:: c

   int regexvm_iter (regexvm_t *compiled, char *input, char **start, char **end, int flags)

Performs an iterative execution of the VM, using the instructions in the
``regexvm_t`` type pointed to by ``compiled`` (which must have already been
populated by ``regexvm_compile()``) and the input string ``input``.

|

**Returns** 1
if the input string contains a substring that matches the expression and 0 if
the input string contains no matching substrings. If a matching substring is
found, the supplied pointers, pointed to by ``start`` and ``end``, will be
populated with the location within the input string where the matching portion
start and ends, respectively. ``start`` and ``end`` will be set to NULL if no
matching substring is found.

|

This function returns after the first matching substring is found, however the
input string can easily be searched for further matches by calling
``regexvm_iter()`` again and passing the ``end`` pointer from the previous
successful invocation as the new ``input`` pointer.

|

|

``regexvm_free``
~~~~~~~~~~~~~~~~

.. code:: c

   void regexvm_free (regexvm_t *compiled)

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

``regexvm_match`` and ``regexvm_iter`` take a ``flags`` parameter. You can use
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
match as many characters as possible, e.g. running ``regexvm_iter`` with
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
    gcc regexvm_iter.c -o regexvm_iter -I/usr/local/include/libregexvm -lregexvm
    gcc regexvm_match.c -o regexvm_match -I/usr/local/include/libregexvm -lregexvm
