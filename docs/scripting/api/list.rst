Lists
=====

This page documents the ``List`` type.

General concepts
----------------

A list is an ordered collection of values, which may be of different types. Elements in a list can be added and removed, and the list grows dynamically to accomodate 
new incoming elements. Indices in a list start at 1 and can be negative: -1 represents the last element, -2 the second-to-last element, and so on.


Methods
-------

.. class:: List


.. method:: append(elem [, ...])

Appends one or more elements at the end of the list.

See also: :func:`prepend`, :func:`insert`

------------

.. method:: clear()

Empty the content of the list. After this method is called, a call to :func:`is_empty` will return ``true``.

------------

.. method:: clone()

Returns a shallow copy of the list.

------------

.. method:: concat(arg [,...])

Returns a new list, which is the concatenation of this list with one or more lists.

------------

.. method:: contains(elem)

Returns ``true`` if elem is in the list and ``false`` otherwise.

------------

.. method:: every(callback)

Returns ``true`` if all the elements in the list satisfy the condition ``callback``, which must be a function that takes a value as input and 
returns a ``Boolean``.

.. code:: phon

    function greater_than_10(x)
        return x > 10
    end
    var lst = [11, 12, 13]

    # Prints "true"
    print(lst.every(greater_than_10))
    

See also: :func:`some`


------------

.. method:: filter(callback)

Returns a new list containing the elements that satisfy the condition ``callback``, which must be a function that takes a value as input and 
returns a ``Boolean``.

.. code:: phon

    function is_odd(x)
        return x % 2 == 1
    end

    var lst = [1, 2, 3, 4, 5, 6]
    print(lst.filter(is_odd)) # prints [1, 3, 5]

See also: :func:`map`

------------

.. method:: find(elem [, i])


Returns the index of ``elem`` in the list, starting the search at index ``i`` (1 by default). If the element is not found,
0 is returned.

Note: If the list is sorted, you can use :func:`sorted_find` instead, which is a little faster since it can take advantage of the fact
that the order of the elements is known.

See also: :func:`find_back`, :func:`sorted_find`

------------


.. method:: find_back(elem [, i])


Returns the index of ``elem`` in the list, starting the search from the end at index ``i`` (-1 by default). If the element is not found,
0 is returned.

See also: :func:`find`

------------

.. method:: includes(other)

Returns ``true`` if the elements in ``other`` are a subsequence this list. Both lists must be sorted, but the elements in ``other`` don't need
to be a contiguous sequence.


------------

.. method:: insert(i, elem)

Inserts the element ``elem`` at index ``i``.

See also: :func:`sorted_insert`


------------

.. method:: intersect(other)

Returns a new list which contains all the elements that are in this list and in ``other``.

See also:: :func:`unite`, :func:`subtract`

------------

.. method:: is_empty()

Returns ``true`` if the list doesn't contain any element, and ``false`` if it does.

------------

.. method:: is_sorted()

See also: :func:`sort`, :func:`reverse`

------------

.. method:: join(sep)

Returns a string in which all elements have been joined with the ``sep`` separator.

------------

.. function:: map(callback)

Returns a new list in which ``callback`` has been applied to each element in the original list. ``callback`` must be a function that takes a value 
and returns a value. 

.. code:: phon

    function upper(s)
        return s.to_upper()
    end
    var lst = ["a", "b", "c"]
    print(lst.map(upper))


See also: :func:`filter`

------------

.. method:: pop()

Removes the last element from the list and returns it.

See also: :func:`shift`

------------

.. method:: prepend(elem [, ...])

Inserts one or more elements at the beginning of the list. 

See also: :func:`append`, :func:`insert`

------------

.. method:: reduce(callback)

Reduces the list to a single value, by applying ``callback`` to each element in the list from beginning to end. ``callback`` must be a function which takes two 
arguments (an accumulator and a value) and returns a single value. 

.. code:: phon

    function callback(accumulator, value)
        return accumulator + value
    end

    var lst = [1, 2, 3, 4]
    print(lst.reduce(callback)) # prints 10

See also: :func:`reduce_back`

------------

.. method:: reduce_back(callback)

Reduces the list to a single value, by applying ``callback`` to each element in the list from end to beginning. ``callback`` must be a function which takes two 
arguments (an accumulator and a value) and returns a single value. 

See also: :func:`reduce`

------------

.. method:: remove(value)

Removes all the elements in the list that are equal to ``value``.


See also: :func:`remove_at`

------------

.. method:: remove(i)

Remove the element at index ``i``.


See also: :func:`remove`

------------


.. method:: reverse()

Reverses the order of the elements in the list. If the elements are not sorted, the result is undefined. (Use :func:`sort` to sort the elements.)


See also: :func:`is_sorted`, :func:`sort`

------------

.. method:: sample(n)

Returns a list containing ``n`` elements from the list drawn at random.

------------

.. method:: shift()

Removes the first element from the list and returns it.

See also: :func:`pop`

------------

..method:: shuffle()

Randomizes the order of elements in the list. 

------------

.. method:: slice(from, to)

Returns a new list which contains the elements of the original list starting at index ``from`` and ending at index ``to`` (inclusive).


------------

.. method:: some(callback)

Returns ``true`` if at least one element in the list satisfies the condition ``callback``, which must be a function that takes a value as input and 
returns a ``Boolean``.

.. code:: phon

    function less_than_10(x)
        return x < 10
    end
    var lst = [9, 11, 12, 13]

    # Prints "true"
    print(lst.every(greater_than_10))
    

See also: :func:`every`

------------

.. method:: sort()

Sorts the elements in the list in increasing order. The elements should be of the same type.

See also: :func:`is_sorted`, :func:`reverse`


.. method:: sorted_find(value)

Finds the index of ``value`` in a sorted list. If ``value`` is not in the list, 0 is returned. Note that if the list is not sorted, the result of this operation is undefined.

This method is faster than :func:`find` on average for sorted lists.

See also: :func:`find`

------------

.. method:: sorted_insert(value)

Inserts ``value`` after the first element that is not less than elem. If the list is not sorted, the result of this operation is undefined.

See also: :func:`insert`

------------

.. method:: subtract(other)

Returns a new list which contains all the elements that are in this list but not in ``other``.

See also:: :func:`intersect`, :func:`unite`

------------

.. method:: to_string()

Returns a string representation of the list.

------------

.. method:: unite(other)

Returns a new list which contains all the elements that are either in this list or in ``other`` (or in both).

See also:: :func:`intersect`, :func:`subtract`


Fields
------

.. attribute:: length

Returns the number of elements in the list.
