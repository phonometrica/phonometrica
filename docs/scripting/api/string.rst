String processing
=================

This page documents the ``String`` type.

General concepts
----------------

A ``String`` is a sequence of characters enclosed between double quotes,
such as ``"this"``. Strings in Phonometrica are immutable, which means that you
cannot modify them directly. All functions which "modify" a string
actually return a new (modified) version of the string but leave the
original string unchanged.

All string functions assume that strings are encoded according to the
UTF-8 `Unicode <http://www.unicode.org>`_ standard. A good tutorial
about UTF-8 can be found at the following address:
`http://www.zehnet.de/2005/02/12/unicode-utf-8-tutorial <http://www.zehnet.de/2005/02/12/unicode-utf-8-tutorial>`_.
In the remainder of this document, the term *character* is used to mean
*extended grapheme cluster* in the sense of the Unicode specification. This generally corresponds to the notion
of "user-perceived character".


Methods
-------


.. class:: String



.. method:: at(pos)

Get character at position ``pos``. If ``pos`` is negative, counting starts from the end.


------------

.. method:: concat(other)

Create a new string which is the concatenation of ``this`` and ``other``.
Another, simpler way to concatenate strings is to use the operator ``+``.


------------

.. method:: contains(substring)

Returns true if the string contains ``substring``, and ``false``
otherwise.


------------

.. method:: count(substring)

Returns the number of times ``substring`` appears in the string.

.. code:: phon

    var s = "cacococococa"
    var count = s.count("coco")

    print(count) # prints "2"

Note: matches don't overlap.


------------

.. method:: ends\_with(suffix)

Returns true if the string ends with ``suffix``, and ``false`` otherwise.

See also: :func:`starts\_with`



------------

.. method:: insert(pos, other)

Returns a copy of the string with ``other`` inserted at position ``pos``


------------

.. method:: left(n)

Get the substring corresponding to the ``n`` first characters of the
string.

------------

.. method:: ltrim()

Returns a copy of the string with whitespace characters removed at the
left end of the string.

.. code:: phon

    var s = "  hello  "

    s = s.ltrim()
    print("$" + s + "$") # prints "$hello  $"

See also: :func:`trim`, :func:`rtrim`


------------

.. method:: mid(from, to)

Returns the substring of ``str`` starting at index ``from`` and ending
at index ``to`` (inclusive). If ``to`` equals ``-1``, returns the
substring from ``from`` until the end of the string.

.. code:: phon

    var s = "c'était ça"

    print(s.mid(3, 7)) # "était"
    print(s.mid(3,-1)) # "était ça"


------------

.. method:: remove(substr)

Returns a copy of the string where all (non-overlapping) instances of the
substring ``substr`` have been removed.

See also: :func:`remove\_at`,
:func:`remove\_first`,
:func:`remove\_last`


------------

.. method:: remove\_at(at, count)

Returns a copy of the string where ``count`` code points, starting at
position ``at``, have been removed.

See also: :func:`remove`,
:func:`remove\_first`,
:func:`remove\_last`


------------

.. method:: remove\_first(substr)

Returns a copy of the string where the first instance of ``substr`` has
been removed.

See also: :func:`remove\_at`, :func:`remove`,
:func:`remove\_last`


------------

.. method:: remove\_last(substr)

Returns a copy of the string where the last instance of ``substr`` has been
removed.

See also: :func:`remove\_at`, :func:`remove`,
:func:`remove\_first`


------------

.. method:: replace(old, new)

Returns a copy of the string where all (non-overlapping) instances of the
substring ``old`` have been replaced by ``new``.

See also: :func:`replace\_at`,
:func:`replace\_first`,
:func:`replace\_last`


------------

.. method:: replace\_at(at, count, new)

Returns a copy of the string where ``count`` code points, starting at
position ``at``, have been replaced by ``new``.

See also: :func:`replace`,
:func:`replace\_first`,
:func:`replace\_last`


------------

.. method:: replace\_first(str, old, new)

Returns a copy of the string where the first instance of the substring
``old`` has been replaced by ``new``.

See also: :func:`replace\_at`,
:func:`replace`, :func:`replace\_last`


------------

.. method:: replace\_last(str, old, new)

Returns a copy of the string where the last instance of the substring
``old`` has been replaced by ``new``.

See also: :func:`replace\_at`,
:func:`replace`, :func:`replace\_first`


------------

.. method:: reverse()

Returns a new string with all the characters in the string in reversed
order.



------------

.. method:: right(n)

Get the substring corresponding to the ``n`` last characters of the
string.


------------

.. method:: rtrim()

Returns a copy of the string with whitespace characters removed at the
right end of the string.

.. code:: phon

    var s = "  hello  "

    s = s.rtrim()
    print("$" + s + "$") # prints "$  hello$"

See also: :func:`ltrim`, :func:`trim`


------------

.. method:: split(delim)

Returns a table of strings which have been split at each occurrence of
the substring ``delim``. If ``delim`` is the empty string, it returns a
list of the characters in the string.


------------

.. method:: starts\_with(prefix)

Returns true if the string starts with ``prefix``, and ``false`` otherwise.

See also: :func:`ends\_with`


------------

.. method:: to\_lower()

Returns a copy of the string where each code point has been converted to
lower case.

.. code:: phon

    var s1 = "C'ÉTAIT ÇA"
    var s2 = s1.to_lower()

    print(s2) # prints "c'était ça"

See also: :func:`to\_upper`


------------

.. method:: to\_upper()

Returns a copy of the string where each code point has been converted to
upper case.

.. code:: phon

    var s1 = "c'était ça"
    var s2 = s1.to_upper()

    print(s2) # prints "C'ÉTAIT ÇA"

See also: :func:`to\_lower`


------------

.. method:: trim()

Returns a copy of the string with whitespace characters removed at both
ends of the string.

.. code:: phon

    var s = "\t  hello  \n"

    s = s.trim()
    print("$" + s + "$") # prints "$hello$"

See also: :func:`ltrim`, :func:`rtrim`






Fields
------


.. attribute:: length

Returns the length of the string, in Unicode extended grapheme clusters.

.. code:: phon

    var s = "안녕하세요"
    print(s.length) # Prints "5"

