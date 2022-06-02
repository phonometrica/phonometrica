Values and references
=====================


.. _clonability:

Clonability
-----------

In computer programming, it is often necessary to assign the content of a variable to another variable. An important question that arises is
"what happens when one of the variables is modified?". Different languages solve this problem in different ways. In Phonometrica, the answer 
depends on the type of the value. Most types in Phonometrica are *clonable*, which means that it makes sense for such types to make a copy of
a value, and two values that have the same content and type should be considered identical. While this is true in most programming languages for 
basic types such as ``Float`` and ``String``, Phonometrica extends this notion to other types such as ``List`` and ``Table``:

.. code:: phon

    n1 = 2.178
    n2 = 2.178
    assert n1 == n2

    s1 = "hello"
    s2 = "hello"
    assert s1 == s2

    lst1 = ["a", "b", "c"]
    lst2 = ["a", "b", "c"]
    assert lst1 == lst2


When we assign a variable that contains a clonable value to another variable, Phonometrica will *make a copy of the original value*. This 
ensures that modifying one variable will not affect the other. Consider the following example:

.. code:: phon

    s1 = "hello"
    s2 = s1
    print s1 # prints "hello"
    print s2 # prints "hello"
    append(s1, " world!")
    print s1 # prints "hello world!"
    print s2 # prints "hello"


As we can see, at first ``s1`` and ``s2`` have the same value, but after we modify ``s1``, ``s2`` retains the same value. Types that behave in
this way are said to have *value semantics*. All clonable types in Phonometrica have value semantics, and most buitin types are clonable. 
The only two exceptions are ``Regex`` and ``File``, which are *non-clonable*. Non-clonable types have *reference semantics*, which means that 
when we assign a variable to another one, they share the same value: modifying one modifies the other:

.. code:: phon

    re1 = Regex("(..)(..)(..)")
    re2 = re1

    if (match(re1, "056559")) then
        print group(re2, 1), "_", group(re2, 2), "_", group(re2, 3)
    end
    
As you can, ``re1`` and ``re2`` refer to the same regular expression, so these variables can be considered as two *aliases* for the same value. 



References
----------

While most programming languages have reference semantics for non-primitive types such as ``List`` and ``Table``, Phonometrica is not an 
isolated case since there are a number of languages that have value semantics by default, including R, MATLAB, PHP, Swift and C++, to name a few. 

Value semantics makes it easier to reason about your code and can prevent a number of subtle bugs because modifying a variable in one place
will not affect variables with the same value in other places. However, in some circumstances, we do need variables to share the same value,
even if the type is clonable. To do this in Phonometrica, you must explicitly declare a variable as a reference using the keyword ``ref``. 
Consider the following example:

.. code:: phon

    s1 = "hello"
    s2 = ref s1
    print s1 # prints "hello"
    print s2 # prints "hello"
    append(s1, " world!")
    print s1 # prints "hello world!"
    print s2 # prints "hello world!"


Here, we declare ``s2`` as a reference to ``s1`: this means that ``s1`` and ``s2`` are two aliases for the same value; modifying one will 
modify the other.

References can also be used when you need to modify the element of a collection. Observe the difference between ``x`` and ``y`` in the 
following example:


.. code:: phon

    lst = ["a", "b", "c"]
    x = lst[1]
    x = to_upper(x)
    print lst # ["a", "b", "c"]
    y = ref lst[2]
    y = to_upper(y)
    print lst # ["a", "B", "c"]


You can also use references to modify the elements of a collection in a ``foreach`` loop:

.. code:: phon

    lst = ["a", "b", "c"]

    foreach ref value in lst do
        value = to_upper(value)
    end

    print lst # ["A", "B", "C"]

Note however that you can only take references to values, not to keys or indexes. The following example will throw a syntax error
because the keys in a table are immutable:

.. code:: phon

    tab = {"name": "John", "surname": "Smith"}

    foreach ref key, val in tab do 
        key = to_upper(key)
    end

You can nevertheless modify the values:

.. code:: phon

    tab = {"name": "John", "surname": "Smith"}

    foreach key, ref val in tab do 
        val = to_upper(val)
    end

    print tab # {"name": "JOHN", "surname": "SMITH"}


References to non-clonable types
--------------------------------

Non-clonable types have reference semantics, so you might wonder if it is possible to create a reference to a reference type. 
The answer is "yes!". Remember that when you create a reference with the keyword ``ref``, what you are doing is simply creating an *alias* 
(i.e another name) for the same object. This means that modifying the content of a variable will modify the other. Observe the following
example:

.. code:: phon

    re1 = Regex("^(.).*")
    re2 = ref re1
    re3 = re1

    re1 = "a string"

    print type(re1) # prints <class String>
    print type(re2) # prints <class String>
    print type(re3) # prints <class Regex>


In this example, the three variables initially point to the same object, but ``re1`` and ``re2`` are two different names for the same variable. 
When we assign a string to ``re1``, ``re2`` is modified as well because for all intents and purposes, ``re2`` *is* ``re1``. On the other, 
``re3`` is a different variable, and as a result it retains its original value. After we modify ``re1``, the regular expression is no longer
shared and ``re3`` is the sole "owner" of the regular expression. 


Avoid references!
-----------------

You might be tempted to use references to avoid the "cost" of copying a value. Don't do that! Phonometrica's scripting language is optimized 
for values, and references are in fact (marginally) more expensive than values. Phonometrica will ensure that no copying occurs unless it 
is really necessary. Use references when you need reference semantics. Everywhere else, use values! 
