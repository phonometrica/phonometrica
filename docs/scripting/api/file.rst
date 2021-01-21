File manipulation
=================

This page documents the ``File`` type, which can be used to read and write text files. ``File`` is :ref:`non-clonable <clonability>`.


General concepts
----------------

This type provides functions to read and create files. The default encoding is UTF-8. Other supported included are UTF-16 and UTF-32 (read-only). 




Global functions
----------------

.. function:: read_file(path)

Return the content of the file named ``path`` as a string.



Methods
-------


.. class:: File

.. method:: init(path as String)


Opens the file named ``path`` and returns a handle to it.  The file is opened for reading: Phonometrica will try to guess the encoding and will default to UTF-8 otherwise.



------------

.. method:: init(path as String, mode as String)


Opens the file named ``path`` and returns a handle to it. The option ``mode`` must be one of the following strings:

* ``"r"`` = open the file in reading mode, starting at the beginning of the file  (the file must exist)
* ``"w"`` = open the file in writing mode, starting at the beginning of the file (the file is overwritten if it already exists)
* ``"a"`` = open the file in reading mode, starting at the end of the file (the file is created if it doesn't exist)
* ``"r+"`` = open the file in reading and writing mode, starting at the beginning of the file (the file must exist)
* ``"w+"`` = open the file in reading and writing mode, starting at the beginning of the file (the file is overwritten if it already exists)
* ``"a+"`` = open the file in reading and writing mode, starting at the end of the file  (the file is created if it doesn't exist)

In reading mode, Phonometrica will try to guess the encoding and will default to UTF-8 otherwise. In writing mode, Phonometrica will always use UTF-8.


------------

.. method:: init(path as String, mode as String, encoding as String)


Opens the file named ``path`` and returns a handle to it. The option ``mode`` must be one of the following strings:

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

Note that the only supported encoding in writing mode is UTF-8.


Functions
---------

------------

.. function:: close(file as File)

Closes the file. Once the file is closed, no further reading or writing operations are allowed. In general, you don't need to call this function since a file is automatically 
closed as soon as the last reference to it released. 


------------

.. function:: eof(file as File)

Returns ``true`` if the cursor is positionned at the end of the file, ``false`` otherwise.


------------

.. function:: open(path as String)


Opens the file named ``path`` and returns a handle to it.  The file is opened for reading: Phonometrica will try to guess the encoding and will default to UTF-8 otherwise.



------------

.. function:: open(path as String, mode as String)


Opens the file named ``path`` and returns a handle to it. The option ``mode`` must be one of the following strings:

* ``"r"`` = open the file in reading mode, starting at the beginning of the file  (the file must exist)
* ``"w"`` = open the file in writing mode, starting at the beginning of the file (the file is overwritten if it already exists)
* ``"a"`` = open the file in reading mode, starting at the end of the file (the file is created if it doesn't exist)
* ``"r+"`` = open the file in reading and writing mode, starting at the beginning of the file (the file must exist)
* ``"w+"`` = open the file in reading and writing mode, starting at the beginning of the file (the file is overwritten if it already exists)
* ``"a+"`` = open the file in reading and writing mode, starting at the end of the file  (the file is created if it doesn't exist)

In reading mode, Phonometrica will try to guess the encoding and will default to UTF-8 otherwise. In writing mode, Phonometrica will always use UTF-8.


------------

.. function:: open(path as String, mode as String, encoding as String)


Opens the file named ``path`` and returns a handle to it. The option ``mode`` must be one of the following strings:

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

Note that the only supported encoding in writing mode is UTF-8.


------------

.. function:: rewind(file as File)

Rewinds the cursor to the beginning of the file.

------------

.. function:: len(file as File)

Returns the number of bytes in the file.

------------

.. function:: write(file as File, text as String)

Writes ``text`` to ``file``.

------------

.. function:: write_line(file as File, text as String)

Writes ``text`` to ``file``, and appends a new line separator.

------------

.. function:: write_lines(file as File, lines as List)

Writes each string in ``lines`` to ``file``, and appends a new line separator after each of them.


------------

.. function:: read_line(file as File)

Reads a line from ``file``. If the cursor is at the end of the file, it returns a empty string.

------------

.. function:: read_lines(file as File)

Returns the content of the file as a list whose elements are the lines of the file.


------------

.. function:: seek(file as File, pos as Integer)

Sets the position of the cursor in the file to ``pos``.


------------

.. function:: tell(file as File)

Returns the current position of the cursor in the file.


Fields
------

.. attribute:: length

Returns the number of bytes in the file.

See also: :func:`len`,