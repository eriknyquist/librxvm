rxvm: non-backtracking NFA-based regular expression engine
==========================================================

.. contents:: Table of Contents

Introduction
------------

``librxvm`` is a **R**\ egular e\ **X**\ pression **V**\ irtual **M**\ achine.
It compiles a regular expression into an NFA representation consisting of a
sequence of primitive opcodes for a "`virtual machine <https://swtch.com/~rsc/regexp/regexp2.html>`_"
(the "machine" here is theoretical, rather than physical hardware, and is
implemented as  a simple bytecode interpreter). The virtual machine can then
execute the NFA representation, or "program" against some input text to
determine whether the input text matches the regular expression.

In addition to the usual string matching & searching functions, librxvm also
provides a function (``fsearch``) that operates directly on a file handle.
``fsearch`` uses the Boyer-Moore-Horspool algorithm to achieve **extremely
high** throughput for regular expression searches on large files.

What's missing from ``librxvm``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Currently, ``librxvm`` only works with plain ol' ASCII.
* It's not POSIX compliant, or anything compliant as far as I know.
* Probably a lot of things.

Quick example
-------------

This example searches a file for all occurrences of the expression
``erik+(nyq|uist?)``:

.. code:: python

    import rxvm

    r = rxvm.compile("erik+(nyq|uist?)")

    with open('file.txt', 'r') as fh:
        while True:
            match = r.fsearch(fh)

            # fsearch returns None when we are at the
            # end of the file
            if not match:
                break

            print 'Found match: %s' % match

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

Installing librxvm
^^^^^^^^^^^^^^^^^^

Use ``pip`` to install

::

    pip install rxvm

Reference
---------

|

``rxvm.compile``
^^^^^^^^^^^^^^^^

.. code:: python

   rxvm.compile (expression)

Compiles the regular expression ``expression`` into NFA form

|

**Return value**

* rxvm.Rxvm object

**Exceptions**

* See section `rxvm.compile exceptions`_


|

|

``rxvm.Rxvm.match``
^^^^^^^^^^^^^^^^^^^

.. code:: python

   rxvm.Rxvm.match (input, flags=0)

Checks if the string ``input`` matches the compiled expression exactly. 'flags'
can be a combination of bitwise-OR'd options from the `Flags`_ section (
``rxvm.MULTILINE`` and ``rxvm.NONGREEDY`` are ignored by ``rxvm.Rxvm.match``)

|

**Return value**

* True if the input matches the expression, False otherwise

|

|

``rxvm.Rxvm.search``
^^^^^^^^^^^^^^^^^^^^

.. code:: python

   rxvm.Rxvm.search (input, flags=0, index=0)

Finds the next occurrence of the compiled expression in ``input``, starting
from ``index``. If a match is found, returns a tuple containing the matching
text, and the index of the first character after the matching text in the input
string. 'flags' can be a combination of bitwise-OR'd options from the `Flags`_
section.

|

**Return value**

* ``(match, index)`` if a match is found, were ``match`` is the matching text,
  and ``index`` is the index of the next character after the matching text. If
  no match is found, then ``(None, index)`` is returned, where ``index`` is the
  index value that was passed.

|

|

``rxvm.Rxvm.fsearch``
^^^^^^^^^^^^^^^^^^^^^

.. code:: python

   rxvm.Rxvm.fsearch (file, flags=0)

Finds the next occurence of the compiled expression in file handle ``file``.
If a match is found, the matching text is returned, and file pointer is
positioned at the next character after the matching text. 'flags' can be a
combination of bitwise-OR'd options from the `Flags`_ section.

This function uses an implementation of the Boyer-Moore-Horspool (BMH) algorithm
to search the file for a pattern, and can be extremely fast. Because the
BMH algorithm only works with fixed strings, this function uses a special
heuristic to identify subtrings of fixed literal characters in your expression,
and uses the fast BMH algorithm to search for these smaller substrings. If one
is found, the virtual machine is invoked (needed to match a regular expression,
but slower).

This means the type of expression you write can significantly affect the speed
of the ``rxvm.Rxvm.fsearch`` function. Specifically, **longer** strings means
**faster** matching.


**Return value**

* If a match is found, the matching text is returned. If no match is found,
  or if the entire file has been read, None is returned.

**Exceptions**

* See `rxvm.Rxvm.fsearch exceptions`_ and `General exceptions`_

|

|

``rxvm.Rxvm.gen``
^^^^^^^^^^^^^^^^^

.. code:: python

   rxvm.Rxvm.gen (generosity=50, whitespace=10, limit=1000)

Generates a pseudo-random string that matches the compiled expression. The
following parameters provide some control over the randomness:

* ``generosity``: This value is expected to be between 0-100, and represents the
  probability out of 100 that a ``+`` or ``*`` operator will generate another
  matching character ("greedyness" in reverse). Higher means more repeat
  matches.
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

**Return value**

* the matching text

**Exceptions**

* See `General exceptions`_

|

|


Flags
-----

``rxvm.Rxvm.match``, ``rxvm.Rxvm.search`` and ``rxvm.Rxvm.fsearch``take a
``flags`` parameter. You can use the masks below to set bit-flags which will
change the behaviour of these functions (combine multiple flags by bitwise
OR-ing them together):

|

``rxvm.ICASE``
^^^^^^^^^^^^^^

case insensitive: ignore case when matching alphabet characters. Matching is
case-sensitive by default.

``rxvm.NONGREEDY``
^^^^^^^^^^^^^^^^^^

non-greedy matching: by default, the operators ``+``, ``*``, and ``?`` will
match as many characters as possible, e.g. running ``rxvm.Rxvm.search`` with
the expression ``<.*>`` against the input string ``<tag>name<tag>`` will match
the entire string. With this flag set, it will match only ``<tag>``.

``rxvm.MULTILINE``
^^^^^^^^^^^^^^^^^^

Multiline: By default, ``^`` matches immediately following the start of input,
and ``$`` matches immediately preceding the end of input or the newline before
the end of input. With this flag set, ``^`` will also match immediately
following each newline character, and ``$`` will also match immediately
preceding each newline character. This flag is ignored and automatically
enabled when ``rxvm.Rxvm.match`` is used; since ``rxvm.Rxvm.match`` effectively
requires a matching string to be anchored at both the start and end of input,
then ``^`` and ``$`` are only useful if they can also act as line anchors.

General exceptions
------------------

The following error codes are returned by all ``librxvm`` functions

|

``rxvm.MemoryAllocationError``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that memory allocation failed.

|

``rxvm.InvalidParameter``
^^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that an invalid parameter (e.g. a ``NULL`` pointer) was passed to a
``librxvm`` library function.

|

``rxvm.compile`` exceptions
---------------------------

The following error codes are returned only by the ``rxvm.compile`` function

|

``rxvm.BadOperator``
^^^^^^^^^^^^^^^^^^^^

Indicates that an operator (``*``, ``+``, ``?``, ``{}``) was used incorrectly
in the input expression, i.e. without a preceding literal character.

|

Example expressions: ``ab++``, ``{5}``.

|

``rxvm.BadClass``
^^^^^^^^^^^^^^^^^

Indicates that an unexpected (and unescaped) character class closing character
(``]``) was encountered in the input expression.

|

Example expressions: ``xy]``, ``[a-f]]``

|

``rxvm.BadRepetition``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that an unexpected (and unescaped) repetition closing character
(``}``) was encountered in the input expression.

|

Example expressions: ``a}``, ``bb{4,}}``

|

``rxvm.BadParenthesis``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that an unexpected (and unescaped) closing parenthesis character
(``)``) was encountered in the input expression.

|

Example expressions: ``qy)``, ``q*(ab))``

|

``rxvm.MissingClosingParenthesis``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that an unterminated parenthesis group (``()``) was encountered in
the input expression.

|

Example expressions: ``d+(ab``, ``((ab)``

|

``rxvm.MissingClosingClass``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that an unterminated character class (``[]``) was encountered in
the input expression.

|

Example expressions: ``[A-Z``, ``[[A-Z]``

|

``rxvm.MissingClosingRepetition``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that an unterminated repetition (``{}``) was encountered in
the input expression.

|

Example expressions: ``ab{5``, ``((ab)``

|

``rxvm.IncompleteCharacterRange``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that an incomplete character range inside a character class was
encountered in the input expression.

|

Example expressions: ``[A-]``, ``[-z]``

|

``rxvm.ExtraRepetitionComma``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that an invalid extra comma inside a repetition was encountered in
the input expression.

|

Example expressions: ``ab{5,,}``, ``x{6,7,8}``

|

``rxvm.NonNumericRepetition``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that an invalid character (i.e. not a digit or a comma) inside a
repetition was encountered in the input expression.

|

Example expressions: ``ab{3,y}``, ``b{8.9}``

|

``rxvm.EmptyRepetition``
^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that an empty repetition (``{}``) was encountered in
the input expression.

|

Example expressions: ``ab{}``, ``ab{,}``

|

``rxvm.TrailingEscape``
^^^^^^^^^^^^^^^^^^^^^^^

Indicates that a trailing escape character (``\\``) was encountered in
the input expression.

|

Example expressions: ``ab\\``, ``\\*\\``

|

``rxvm.InvalidSymbol``
^^^^^^^^^^^^^^^^^^^^^^

Indicates that an invalid symbol (any character outside the supported
character set) was encountered in the input expression.

|

``rxvm.Rxvm.fsearch`` exceptions
--------------------------------

The following error codes are returned only by the ``rxvm.Rxvm.fsearch``
function

|

``rxvm.IOError``
^^^^^^^^^^^^^^^^

Indicates that an error occured while attempting to read from the passed
file

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
