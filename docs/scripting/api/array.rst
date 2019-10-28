Numeric arrays
==============

The ``Array`` type represents a numeric array with one or two dimensions. A One-dimension array, or **vector**, is just a
 special case of a two-dimension array (i.e. a **matrix**): it is treated as a matrix with a single column (it is a **column vector**).
Elements in a matrix are accessed with a set of two indices, noted *(i, j)*, where *i* represents the *i*th row and *j*
 represents the *j*th column. Indices in each dimension start at 1 and can be negative. (Negative indices start from the end of the dimension.)
 For example, index *(3, 2)* represents the item in the third row in the second column, whereas *(1, -1)* represents element
 in the first row and in the last column.


Methods
-------


.. class:: Array

.. method:: Array(nrow [, ncol])

Constructs a matrix with ``nrow`` rows and ``ncol`` columns. If ``ncol`` is not specified, it is equal to 1. All elements are initialized to 0.

------------

.. method:: get(i [, j])

Returns the value at row ``i`` and column ``j``. If ``j`` is not specified, it is equal to 1.

------------

.. method:: set(i [, j], value)

Sets the element at row ``i`` and column ``j`` with the given ``value``. If ``j`` is not specified, it is equal to 1.

------------

.. method:: transpose()

Returns the transpose of the matrix.

------------

.. method:: add(x)

Add ``x`` to all the elements in the array.

------------

.. method:: sub(x)

Subtracts ``x`` from all the elements in the array.


Fields
------

.. attribute:: length

Returns the number of elements in the array.

.. attribute:: row_count

Returns the number of rows in the array.

.. attribute:: column_count

Returns the number of columns in the array.