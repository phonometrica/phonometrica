Numeric arrays
==============

The ``Array`` type represents a numeric array with one or two dimensions. Elements along each dimension start at 1 and can be negative. 
(Negative indices start from the end of the dimension.) Two-dimensional arrays are accessed with a pair of indices noted *(i, j)*,
where *i* represents the *i* th row and *j* represents the *j* th column. For example, index *(3, 2)* represents the item in the third 
row in the second column, whereas *(1, -1)* represents element in the first row and in the last column.


Methods
-------


.. class:: Array

.. method:: Array(m [, n])

Constructs a vector with ``m`` elements  or an ``m`` by ``n`` matrix. All elements are initialized to 0.

------------

.. method:: get(i [, j])

Returns the value at index ``i`` in a vector or at index ``(i,j)`` in a matrix.

------------

.. method:: set(i [, j], value)

Sets the element at index ``i`` in a vector or at index ``(i,j)`` in a matrix.

------------

.. method:: transpose()

Returns the transpose of the matrix.

------------

.. method:: add(n)

Returns a new array where ``n`` was added to each element in the original array.

------------

.. method:: sub(n)

Returns a new array where ``n`` was subtracted from each element in the original array.


------------

.. method:: mul(n)

Returns a new array where each element in the original array was multiplied by ``n``.

------------

.. method:: div(n)

Returns a new array where each element in the original array was divided by ``n``.


Fields
------

.. attribute:: length

Returns the number of elements in the array.

.. attribute:: row_count

Returns the number of rows in the array.

.. attribute:: column_count

Returns the number of columns in the array.

.. attribute:: dim_count

Returns the number of dimensions of the array.