File manipulation
=================

This page documents the ``File`` class, which can be used to read and write text files.

General concepts
----------------

This class provides functions to read and create files. The default encoding is UTF-8. Other supported included are UTF-16 and UTF-32 (read-only). 


Methods
-------


.. class:: File

.. method:: File(path [, mode [, encoding]])

Open the file named ``path`` and return a handle to it. If no mode is specified, the file is opened for reading. Otherwise, ``mode`` must be one of the following strings:

* ``"r"`` = open the file in reading mode, starting at the beginning of the file  (the file must exist)
* ``"w"`` = open the file in writing mode, starting at the beginning of the file (the file is overwritten if it already exists)
* ``"a"`` = open the file in reading mode, starting at the end of the file (the file is created if it doesn't exist)
* ``"r+"`` = open the file in reading and writing mode, starting at the beginning of the file (the file must exist)
* ``"w+"`` = open the file in reading and writing mode, starting at the beginning of the file (the file is overwritten if it already exists)
* ``"a+"`` = open the file in reading and writing mode, starting at the end of the file  (the file is created if it doesn't exist)

If ``encoding`` is not specified, Phonometrica will try to guess the encoding in reading mode and will use UTF-8 in writing mode. Otherwise, ``encoding``
must be one of the following strings:

* ``utf8`` = UTF-8 encoding
* ``utf16`` = UTF-16, using the platform's endianness
* ``utf32`` = UTF-32, using the platform's endianness
* ``utf16le`` = UTF-16 little endian
* ``utf16be`` = UTF-16 big endian
* ``utf32le`` = UTF-32 little endian
* ``utf32be`` = UTF-32 big endian


------------

.. function:: close()

Close the file. Once the file is closed, no further reading or writing operations are allowed.

------------

.. function:: eof()

Returns ``true`` if the cursor is positionned at the end of the file, ``false`` otherwise.

------------

.. function:: rewind()

Rewind the cursor to the beginning of the file.

------------

.. function:: size()

Return the number of bytes in the file.

------------

.. function:: write(text)

Write ``text`` into ``file``.

------------

.. function:: write_line(text)

Write ``text`` into ``file``, and append a new line separator.

------------

.. function:: write_byte(byte)

Write a single byte into the file.

------------

.. function:: read_byte()

Read a single byte from the file.

------------

.. function:: read_line()

Read a line from the file file. If the cursor is at the end of the file, return a empty string.

------------

.. method:: read_lines()

Return the content of the file as a table whose elements are the lines of the file.

------------

.. staticmethod:: read_all(path)

Return the content of the file named ``path`` as a string.
