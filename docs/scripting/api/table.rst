Tables
======

This page documents the ``Table`` type. ``Table`` is :ref:`clonable <clonability>`.

General concepts
----------------

A ``Table`` (also known as map, hash map, hash table, associative array or dictionary) is an unordered mapping of key/value pairs. Each key/value pair represents a *field*. Keys can be any clonable value (except ``null``), whereas values can be anything. 
Tables can be declared with a table literal:

.. code:: phon

    person = { "name" : "john", "surname" : "smith", "age" : 38 }

In this example, ``person`` is declared with three pairs separated by commas: the key and the value are separated by the symbol ``:`` (colon). This table could correspond to mappings from names (keys) to ages (values) for instance. Note that there is no need for the keys and/or values to be homogeneous: any valid Value (even null!) may appear in an object. 
Note that even though we declared key/value pairs in a specific order in our example, there is no guarantee that they will be stored in this particular order. You should consider the order of the elements as random.

To create an empty table, you can either use an empty table literal or call call ``Table``'s constructor without any argument:

.. code:: phon

    tab1 = {}
    tab2 = Table()
    assert is_empty(tab1)
    assert is_empty(tab2)


To access any element of a table, you can use the index operator ``[]``:

.. code:: phon

    person = { "name" : "john", "surname" : "smith", "age" : 38 }
    print person["name"]
    person["age"] += 1
    print person



If you need to process the table in sorted order, you can do as follows (assuming you have a table named ``tab``):

.. code:: phon

    keys = tab.keys
    sort(keys)
    foreach key in keys do
        value = tab[key]
        # do something with the key and the value
    end


Methods
-------

.. class:: Table


.. method:: init()

Creates a new empty table.



Functions
---------

.. function:: clear(ref table as Table)

Removes all the elements in the table. 


------------

.. function:: contains(table as Table, key as Object)

Returns ``true`` if there is an element in the table whose key is equal to ``key``, and ``false`` otherwise.


------------

.. function:: get(table as Table, key as Object)

Returns the value associated with ``key`` if there is one, or ``null`` if there is no such value. 
For keys that exist, the behavior of this function is identical to indexing with ``[]``, but observe how the two differ
for missing keys:

.. code:: phon

    tab = { "john": "smith", "hello": "world" }
    print get(tab, "pi") # prints null
    print tab["pi"] # throws an index error


------------

.. function:: get(table as Table, key as Object, default as Object)

Returns the value associated with ``key`` if there is one, or ``default`` if there is no such value. 


------------

.. function:: is_empty(table as Table)

Returns ``true`` if the table contains no element, and ``false`` otherwise.

.. function:: remove(ref table as Table, key as Object)

Removes the element whose key is equal to ``key``. If there is no such element, this function does nothing.



Fields
------

.. attribute:: keys

Returns the keys in the table as a list, in an unspecified order.

------------

.. attribute:: length

Returns the number of elements in the table.

------------

.. attribute:: values


Returns the values in the table as a list, in an unspecified order.


See also:: :func:`len`