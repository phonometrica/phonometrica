Numeric arrays
==============

The ``Array`` type represents a numeric array with one or two dimensions. Array is :ref:`clonable <clonability>`.

Array indexing
--------------

To get or set an element in an array, use the index ``[]`` operator. Elements along each dimension start at 1 and can be negative.
(Negative indices start from the end of the dimension.)
Two-dimensional arrays are accessed with a pair of indices noted *(i, j)*,
where *i* represents the *i* th row and *j* represents the *j* th column. For example, index *(3, 2)* represents the item in the third
row in the second column, whereas *(1, -1)* represents element in the first row and in the last column.

.. code:: phon

    X = new Array(3, 4)
    X[1,4] = 10
    print(X[1,4])



Global functions
----------------

.. function:: read_matrix(path [, separator [, drop_header]])

Reads a two-dimensional numeric array from a CSV file, in which values are separated by ``separator`` (by default, a comma).
If ``drop_header`` is ``true`` (default), the first line will be treated as a header and will be dropped.

------------

.. function:: write_matrix(M, path [, separator])

Writes a two-dimensional numeric array ``M`` to a CSV file, in which values are separated by ``separator`` (by default, a comma).


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

.. method:: get_row(i)

Returns the ``i`` th row as a new one-dimensional array. The original array must be two-dimensional.

See also: :func:`get_column`

------------

.. method:: get_column(j)

Returns the ``j`` th column as a new one-dimensional array. The original array must be two-dimensional.

See also: :func:`get_row`

------------

.. method:: mul(n)

Returns a new array where each element in the original array was multiplied by ``n``.

See also :func:`add`, :func:`sub`, :func:`div`

------------

.. method:: shuffle()

Randomly shuffles the elements of the array. This method modifies the array in place.

------------

.. method:: slice(d1_from, d1_to [, d2_from, d2_to])

Returns a new array which is a slice of the original array. If the original array has one dimension, the result is
a one-dimensional array starting from index ``d1_from`` up to and including ``d1_to``. If the array has two dimensions, the result is a
matrix that includes rows starting from row ``d1_from`` up to and including row ``d1_to``, and columns starting from
``d2_from`` up to and including row ``d2_to``.

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
