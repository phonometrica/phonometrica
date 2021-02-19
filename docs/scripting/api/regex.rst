Regular expressions
===================

This page documents the ``Regex`` type. ``Regex`` is :ref:`non-clonable <clonability>`.

General concepts
----------------

Regular expressions are widely used in text processing to perform
pattern matching and pattern substitution. Simply put, a regular
expression (regex) is a string which describes a *set of strings*.
Suppose that we want to any of the following strings: ``"petit"``,
``"petite"``, ``"petits"``, ``"petites"``. Instead of looking for each
string separately, we can use a regular expression to look for any of
them. The corresponding regular expression would be ``"petite?s?"``.

Syntax
~~~~~~

Regular expressions always try to match a pattern from left to right; in
their simplest form, they match a sequence of (non-special) characters
and are equivalent in this case to a plain text search. Regular
expressions provide a number of special symbols and operators that can
match classes or sequences of characters. Here we only provide the most
useful ones:

-  ``.`` : match any character
-  ``^`` : match the beginning of a string
-  ``$`` : match the end of a string
-  ``[xyz]`` : match either of the characters ``x``, ``y`` or ``z``
-  ``[^xyz]`` : match any character except ``x``, ``y`` or ``z``
-  ``[a-z]`` : match any character in the range from ``a`` to ``z``
-  ``\b`` : match a word boundary
-  ``\s`` : match a white space character
-  ``\d`` : match a digit character (equivalent to ``[0-9]``)
-  ``\w`` : match a word character, including digits and ``_``
   (underscore)

In addition, regular expressions offer a number of quantifiers:

-  ``E?`` : match 0 or 1 occurrences of the expression E
-  ``E*`` : match 0 or more occurrences of the expression E
-  ``E+`` : match 1 or more occurrences of the expression E
-  ``E{n}`` : match exactly n occurrences of the expression E
-  ``E{n,m}`` : match between n and m occurrences of the expression E
-  ``E{n,}`` : match at least n occurrences of the expression E
-  ``E{,m}`` : match at most m occurrences of the expression E (and
   possibly 0)

In this context, an expression must be understood as either a character
(e.g. ``o{2,}`` matches the string ``"zoo"``) or a sequence of
characters enclosed by parentheses (e.g. ``(?:do){2}`` matches the
string ``"fais dodo"``). Another useful character is ``|``, which is
used to combine expressions (logical OR). For example, the pattern
``(?:est|était)`` will find all occurrences of the strings est and
était.

Regular expressions are "greedy" by default, which means they will match
the longest string that satisfies the pattern. For instance, given the
pattern ``j.*e``, which matches the character ``j`` followed by zero or
more characters followed by ``e``, and the string ``"je te l'ai dit"``,
a non-greedy search will return the substring ``"je te"`` by default.
Non-greedy search, on the other hand, will yield the substring ``"je"``
since it extracts the shortest string that satisfies the regular
expression. To enable non-greedy behavior, we must use the quantifier
``?`` after the star (in this case, ``"j.*?e"``).

Constructors
------------

.. class:: Regex

.. method:: Regex(pattern as String)

Create a new regular expression from a string pattern. The regex can be matched against any string.

.. code:: phon

    re = Regex("^(..)")
    # Do something with re...

See also: :func:`pattern`


.. method:: Regex(pattern as String, flags as String)

Create a new regular expression from a string pattern. The ``flags`` argument can contain any of the following
options, separated by the character ``|``:

- ``caseless``: ignore case
- ``multiline``: match expression on several lines

.. code:: phon

    re = Regex("^(..)", "caseless|multiline")
    # Do something with re...

See also: :func:`pattern`


Functions
---------

.. function:: count(regex as Regex)

Returns the number of captures in the last match. This is equivalent to ``len()``.


------------


.. function:: match(regex as Regex, subject as String)

Match ``regex`` against the string ``subject``. Returns
``true`` if there was a match, and ``false`` otherwise.

See also: :func:`count`, :func:`group`, :func:`has_match`


------------


.. function:: match(regex as Regex, subject as String, pos as Integer)

Match ``regex`` against the string ``subject``, starting at position ``pos``. Returns
``true`` if there was a match, and ``false`` otherwise.

See also: :func:`count`, :func:`group`, :func:`has_match`

------------

.. function:: has_match(regex as Regex)

Returns ``true`` if the last call to ``match`` was sucessful, and
``false`` if it was unsuccessful or if ``match`` was not called.

See also: :func:`match`


------------

.. function:: get_end(regex as Regex, nth as Intger)

Returns the index of the last character of the ``nth`` capture in ``regex``. If
``nth`` equals ``0``, it returns the index of the last character in the
whole matched string.

See also: :func:`match`, :func:`get_start`


------------

.. function:: get_start(regex as Regex, nth as Integer)

Returns the index of the first character of the ``nth`` capture in ``regex``. If
``nth`` equals ``0``, it returns the index of the first character in the
whole matched string.

See also: :func:`group`, :func:`get_end`


------------

.. function:: group(regex as Regex, nth as Integer)

Returns the ``nth`` captured sub-expression in the last successful call
to ``match``. If ``nth`` equals ``0``, the whole matched string is
returned, even if no sub-expression was captured.

See also: :func:`count`, :func:`match`, :func:`first`, :func:`last`

------------

.. function:: len(regex as Regex)

Returns the number of captures in the last match. This function returns 0 if there was no captured
sub-expression, if there was no match or if ``match`` was not called. This is equivalent to ``count()``.

.. code:: phon

    re = Regex("^a(...)(..)(..)")

    # Print "bra", "ca", "da"
    if match(re, "abracadabra") then
        for i = 1 to len(re) do
            print group(re, i)
        end       
    end


Fields
------

.. attribute:: length

Returns the number of captured sub-expressions in the last call to
``match``. This field is equal to 0 if there was no captured
sub-expression, if there was no match or if ``match`` was not called.

.. code:: phon

    re = new Regex("^a(...)(..)(..)")

    # Print "bra", "ca", "da"
    if re.match("abracadabra") then
        for i = 1 to re.length do
            text = re.group(i)
            print(text)
        end       
    end

See also: :func:`len`, :func:`count`


------------

.. attribute:: pattern


Returns the pattern (as a ``String``) from which the regular
expression was constructed.