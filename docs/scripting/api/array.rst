Numeric arrays
==============

The ``Array`` type represents a numeric array with one or two dimensions.

Array indexing
--------------

To get or set an element in an array, use the index ``[]`` operator. Elements along each dimension start at 1 and can be negative.
(Negative indices start from the end of the dimension.)
Two-dimensional arrays are accessed with a pair of indices noted *(i, j)*,
where *i* represents the *i* th row and *j* represents the *j* th column. For example, index *(3, 2)* represents the item in the third
row in the second column, whereas *(1, -1)* represents element in the first row and in the last column.

.. code:: phon

    var X = new Array(3, 4)
    X[1,4] = 10
    print(X[1,4])


Methods
-------


.. class:: Array

.. method:: Array(m [, n])

Constructs a vector with ``m`` elements  or an ``m`` by ``n`` matrix. All elements are initialized to 0.


------------

.. method:: add(n)

Returns a new array where ``n`` was added to each element in the original array.

See also :func:`sub`, :func:`mul`, :func:`div`

------------

.. method:: clone()

Returns a deep copy of the array.

------------

.. method:: div(n)

Returns a new array where each element in the original array was divided by ``n``.

See also :func:`add`, :func:`sub`, :func:`mul`

------------

.. method:: mul(n)

Returns a new array where each element in the original array was multiplied by ``n``.

See also :func:`add`, :func:`sub`, :func:`div`

------------

.. method:: shuffle()

Randomly shuffles the elements of the array. This method modifies the array in place.

------------

.. method:: sub(n)

Returns a new array where ``n`` was subtracted from each element in the original array.

See also :func:`add`, :func:`mul`, :func:`div`

------------

.. method:: to_string()

Returns a string representation of the array.

------------

.. method:: transpose()

Returns the transpose of the matrix.



Fields
------

.. attribute:: dim_count

Returns the number of dimensions of the array.

.. attribute:: column_count

Returns the number of columns in the array.

.. attribute:: length

Returns the number of elements in the array.

.. attribute:: row_count

Returns the number of rows in the array.
