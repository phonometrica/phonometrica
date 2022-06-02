String processing
=================

This page documents the ``String`` type. ``String`` is :ref:`clonable <clonability>`.

General concepts
----------------

A ``String`` is a sequence of characters enclosed between single or double quotes,
such as ``'this'`` or ``"this"``. Strings in Phonometrica are mutable, which means that some
functions allow you to modify them directly. 

All string functions assume that strings are encoded according to the
UTF-8 `Unicode <http://www.unicode.org>`_ standard. A good tutorial
about UTF-8 can be found at the following address:
`http://www.zehnet.de/2005/02/12/unicode-utf-8-tutorial <http://www.zehnet.de/2005/02/12/unicode-utf-8-tutorial>`_.
In the remainder of this document, the term *character* is used to mean
*extended grapheme cluster* in the sense of the Unicode specification. This generally corresponds to the notion
of "user-perceived character".


Constructor
-----------


.. class:: String



.. method:: String()

Creates an empty string.



Functions
---------


.. function:: append(ref string as String, suffix as String)

Inserts ``suffix`` as the end of ``string``.

See also: :func:`prepend`

------------

.. function:: char(pos as Integer)

Get character at position ``pos``. If ``pos`` is negative, counting starts from the end.



------------

.. function:: contains(string as String, substring as String)

Returns ``true`` if ``string`` contains ``substring``, and ``false``
otherwise.


------------

.. function:: count(string as String, substring as String)

Returns the number of times ``substring`` appears in ``string``.

.. code:: phon

    s = "cacococococa"
    count = count(s, "coco")

    print count # prints "2"

Note: matches don't overlap.


------------

.. function:: ends_with(string as String, suffix as String)

Returns true if the string ends with ``suffix``, and ``false`` otherwise.

See also: :func:`starts_with`


------------

.. function:: find(string as String, substring as String)

Returns the start position of ``substring`` in ``string``, or 0 if it is not found. Searching
proceeds from left to right.

See also: :func:`find_back`


.. function:: find(string as String, substring as String, pos as Integer)

Returns the start position of ``substring`` in ``string``, or 0 if it is not found. Searching
proceeds from left to right, starting at ``pos``.

See also: :func:`find_back`

------------

.. function:: find_back(string as String, substring as String)

Returns the start position of ``substring`` in ``string``, or 0 if it is not found. Searching
proceeds from right to left.

See also: :func:`find`

------------

.. function:: find_back(string as String, substring as String, pos as Integer)

Returns the start position of ``substring`` in ``string``, or 0 if it is not found. Searching
proceeds from right to left, starting at ``pos``.

See also: :func:`find`

------------

function:: insert(ref string as String, pos as Integer, other as String)

Inserts ``other`` at position ``pos``.

------------

function:: is_empty(string as String)

Returns true if the string is empty.

------------

.. function:: left(string as String, n as Integer)

Get the substring corresponding to the ``n`` first characters of the
string.

------------

.. function:: len(string as String)

Returns the number of characters in the string.

------------

.. function:: ltrim(ref string as String)

Removes whitespace characters at the left end of the string.

.. code:: phon

    s = "  hello  "
    ltrim(s)
    print "$", s, "$" # prints "$hello  $"

See also: :func:`trim`, :func:`rtrim`


------------

.. function:: prepend(ref string as String, suffix as String)

Inserts ``suffix`` as the beginning of ``string``.

See also: :func:`append`

------------

.. function:: remove(ref string as String, sub as String)

Removes all (non-overlapping) instances of the substring ``sub``.

See also: :func:`remove_at`,
:func:`remove_first`,
:func:`remove_last`


------------

.. function:: remove_at(ref string as String, at as Integer, count as Integer)

Removes ``count`` characters, starting at position ``at``.

See also: :func:`remove`,
:func:`remove_first`,
:func:`remove_last`


------------

.. function:: remove_first(ref string as String, sub as String)

Removes the first instance of the substring ``sub``.

See also: :func:`remove_at`, :func:`remove`,
:func:`remove_last`


------------

.. function:: remove_last(ref string as String, sub as String)

Removes the last instance of the substring ``sub``.

See also: :func:`remove_at`, :func:`remove`,
:func:`remove_first`


------------

.. function:: replace(ref string as String, old as String, new as String)

Replaces all (non-overlapping) instances of the substring ``old`` by ``new``.

See also: :func:`replace_at`,
:func:`replace_first`,
:func:`replace_last`


------------

.. function:: replace_at(ref string as String, at as Integer, count as Integer, new as String)

Replaces ``count`` characters starting at position ``at`` with substring ``new``.

See also: :func:`replace`,
:func:`replace_first`,
:func:`replace_last`


------------

.. function:: replace_first(ref string as String, old as String, new as String)

Replaces the first instance of the substring ``old`` with ``new``.

See also: :func:`replace_at`,
:func:`replace`, :func:`replace_last`


------------

.. function:: replace_last(ref string as String, old as String, new as String)

Replaces the last instance of the substring ``old`` with ``new``.

See also: :func:`replace_at`,
:func:`replace`, :func:`replace_first`


------------

.. function:: reverse(ref string as String)


Reverse all characters in the string.

.. code:: phon

    s = "noël"
    reverse(s)
    print s # prints "lëon"


------------

.. function:: right(string as String, n as Integer)

Get the substring corresponding to the ``n`` last characters of the
string.


------------

.. function:: rtrim(ref string as String)

Removes whitespace characters removed at the right end of the string.

.. code:: phon

    s = "  hello  "
    rtrim(s)
    print "$", s, "$" # prints "$  hello$"

See also: :func:`ltrim`, :func:`trim`

------------

.. function:: slice(string as String, from as Integer)

Returns the substring starting at index ``from`` until the end of the string.


------------

.. function:: slice(string as String, from as Integer, to as Integer)

Returns the substring starting at index ``from`` and ending
at index ``to`` (inclusive). If ``to`` equals ``-1``, returns the
substring from ``from`` until the end of the string.

.. code:: phon

    s = "c'était ça"

    print slice(s, 3, 7) # "était"
    print slice(s, 3,-1) # "était ça"

------------

.. function:: split(string as String, delim as String)

Returns a List of strings which have been split at each occurrence of
the substring ``delim``. 


------------

.. function:: starts_with(string as String, prefix as String)

Returns true if the string starts with ``prefix``, and ``false`` otherwise.

See also: :func:`ends_with`


------------

.. function:: to_lower(string as String)

Returns a copy of the string where each character has been converted to
lower case.

.. code:: phon

    s1 = "C'ÉTAIT ÇA"
    s2 = to_lower(s1)

    print s2 # prints "c'était ça"

See also: :func:`to_upper`


------------

.. function:: to_upper(string as String)

Returns a copy of the string where each character has been converted to
upper case.

.. code:: phon

    s1 = "c'était ça"
    s2 = to_upper(s1)

    print s2 # prints "C'ÉTAIT ÇA"

See also: :func:`to_lower`


------------

.. function:: trim(ref string as String)

Removes whitespace characters removed at both ends of the string.

.. code:: phon

    s = "\t  hello  \n"
    trim(s)
    print "$", s, "$" # prints "$hello$"

See also: :func:`ltrim`, :func:`rtrim`


Fields
------


.. attribute:: first

Returns the first character in the string.

------------


.. attribute:: last

Returns the last character in the string.

------------

.. attribute:: length

Returns the number of characters in the string.

.. code:: phon

    s = "안녕하세요"
    print s.length # Prints "5"

See also: :func:`len`    