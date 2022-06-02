Lists
=====

This page documents the ``List`` type. ``List`` is :ref:`clonable <clonability>`.

General concepts
----------------

A list is an ordered collection of values, which may be of different types. Elements in a list can be added and removed, and the list grows dynamically to accomodate 
new incoming elements. Indices in a list start at 1 and can be negative: -1 represents the last element, -2 the second-to-last element, and so on.

Lists can be created using a literal array and elements can be accessed and modified using the indexing operator ``[]``:

.. code:: phon

    var lst = ["A", 111, "hello", 3.14]
    lst[-1] = "pi"
    print lst # print '["A", 111, "hello", "pi"]'


Methods
-------

.. class:: List


.. method:: init()

Creates a new empty list.

.. code:: phon

    # Create a new empty list using ``List``'s constructor (which calls the ``init()`` method)
    var lst1 = List()
    # Create a new empty list using a list literal
    var lst2 = []



Functions
---------

.. function:: append(ref list as List, item as Object)

Inserts ``item`` at the end of ``list``.

See also: :func:`prepend`, :func:`insert`

------------

.. function:: clear(ref list as List)

Empty the content of the list. After this function is called, a call to :func:`is_empty` will return ``true``.

------------

.. function:: contains(list as List, item as Object)

Returns ``true`` if ``item`` is in the list and ``false`` otherwise.

------------

.. function:: find(list as List, item as Object)


Returns the index of ``item`` in the list. If the element is not found, 0 is returned.

Note: If the list is sorted, you can use :func:`sorted_find` instead, which is a little faster since it can take advantage of the fact
that the order of the elements is known.

See also: :func:`find_back`, :func:`sorted_find`

------------

.. function:: find(list as List, item as Object, pos as Integer)


Returns the index of ``item`` in the list, starting the search at index ``pos``. If the element is not found, 0 is returned.

Note: If the list is sorted, you can use :func:`sorted_find` instead, which is a little faster since it can take advantage of the fact
that the order of the elements is known.

See also: :func:`find_back`, :func:`sorted_find`

------------


.. function:: find_back(list as List, item as Object)


Returns the index of ``item`` in the list, starting the search from the end. If the element is not found,
0 is returned.

See also: :func:`find`

------------


.. function:: find_back(list as List, item as Object, pos as Integer)


Returns the index of ``item`` in the list, starting the search from the end at index ``pos``. If the element is not found,
0 is returned.

See also: :func:`find`


------------

.. function:: insert(ref list as List, pos as Integer, item as Object)

Inserts the element ``item`` at index ``pos``.

See also: :func:`sorted_insert`


------------

.. function:: intersect(list1 as List, list2 as List)

Returns a new list which contains all the elements that are in both ``list1`` and ``list2``. This function assumes that the lists 
are sorted.

See also:: :func:`unite`, :func:`subtract`

------------

.. function:: is_empty(list as List)

Returns ``true`` if the list doesn't contain any element, and ``false`` if it does.

------------

.. function:: is_sorted(list as List)

Returns true if all the elements are sorted in ascending order.

See also: :func:`sort`, :func:`reverse`

------------

.. function:: join(items as List, delim as String)

Returns a string in which all the elements in ``items`` have been joined with the separator ``delim``.

------------

.. function:: len(list as List)

Returns the number of elements in the list.

------------

.. function:: pop(ref list as List)

Removes the last element from the list and returns it.

See also: :func:`shift`

------------

.. function:: prepend(ref list as List, item as Object)

Inserts ``item`` at the beginning of the list. 

See also: :func:`append`, :func:`insert`


------------

.. function:: remove(ref list as List, item as Object)

Removes all the elements in the list that are equal to ``item``.


See also: :func:`remove_at`, :func:`remove_first`, :func:`remove_last`


------------

.. function:: remove_first(ref list as List, item as Object)

Removes the first element in the list that is equal to ``item``.


See also: :func:`remove_at`, :func:`remove`, :func:`remove_last`

------------

.. function:: remove_last(ref list as List, item as Object)

Removes the last element in the list that is equal to ``item``.


See also: :func:`remove_at`, :func:`remove`, :func:`remove_first`

------------

.. function:: remove_at(ref list as List, pos as Integer)

Remove the element at index ``pos``.


See also: :func:`remove`, :func:`remove_first`, :func:`remove_last`

------------


.. function:: reverse(ref list as List)

Reverses the order of the elements in the list. If the elements are not sorted, the result is undefined. (Use :func:`sort` to sort the elements.)


See also: :func:`is_sorted`, :func:`sort`

------------

.. function:: sample(list as List, n as Integer)

Returns a list containing ``n`` elements from the list drawn at random.

------------

.. function:: shift(ref list as List)

Removes the first element from the list and returns it.

See also: :func:`pop`

------------

.. function:: shuffle(ref list as List)

Randomizes the order of the elements in the list. 

------------

.. function:: slice(list as List, from as Integer, to as Integer)

Returns a new list which contains the elements of the original list starting at index ``from`` and ending at index ``to`` (inclusive).


------------

.. function:: sort(ref list as List)

Sorts the elements in the list in increasing order. The elements should be of the same type.

See also: :func:`is_sorted`, :func:`reverse`

------------

.. function:: sorted_find(list as List, item as Object)

Finds the index of ``item`` in a sorted list. If ``item`` is not in the list, 0 is returned. Note that if the list is not sorted, the result of this operation is undefined.

This function is generally faster than :func:`find` for sorted lists, as it takes logarithmic (as opposed to linear) time on average.

See also: :func:`find`

------------

.. function:: sorted_insert(ref list as List, item as Object)

Inserts ``item`` after the first element that is not less than elem. If the list is not sorted, the result of this operation is undefined.

See also: :func:`insert`

------------

.. function:: str(list as List)

Returns a string representation of the list.

------------

.. function:: subtract(list1 as List, list2 as List)

Returns a new list which contains all the elements that are in ``list1`` but not in ``list2``. This function assumes that the lists 
are sorted.

See also:: :func:`intersect`, :func:`unite`

------------

.. function:: unite(list1 as List, list2 as List)

Returns a new list which contains all the elements that are in ``list1`` and/or in ``list2``. This function assumes that the lists 
are sorted.

See also:: :func:`intersect`, :func:`subtract`


Fields
------


.. attribute:: first

Returns the first item in the list.

------------

.. attribute:: last

Returns the last item in the list.

------------

.. attribute:: length

Returns the number of elements in the list.

See also:: :func:`len`