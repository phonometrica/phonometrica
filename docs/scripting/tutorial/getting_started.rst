Getting started
===============

Fundamental notions
-------------------

Comments
~~~~~~~~

Single-line comments start with the symbol ``#`` and end at the end of the line.

.. code:: phon

    # Text following the symbol '#' is treated as a comment and is ignored
    # Call the built-in function print()
    print("hello world")

Multi-line comments start with ``!*`` and end with ``!*``. They can span over several lines, but cannot be nested.

.. code:: phon

    !* 
    This is a multi-line comment which is ignored. 
    Call the built-in function print()
    *!
    print("hello world")


Variables
~~~~~~~~~

A variable must start with a letter or the symbol ``_``, and can be followed by any number 
of letters, digits or the symbol ``_``. The name of variables is case-sensitive, which means that ``myvar`` and ``Myvar`` 
are treated as different variables. Variables can be declared using the ``var`` keyword. 


.. code:: phon

    var x = 5
    var y


In this example, the variable ``x`` is declared and is simultaneously assigned the value 5 (a ``Number``), whereas ``y`` is simply declared but is not assigned any value. When a variable is declared but is not explicitly assigned a value, it is automatically initialized to the value ``null``. 


Because Phonometrica's scripting language is dynamic, variables can be bound to values of any type. In the following example, ``x`` is first declared as a number, and is subsequently used to store a string:


.. code:: phon

    var x = 5
    print(x) # prints "5"
    x = "hello"
    print(x) # prints "hello"


Variable scope
~~~~~~~~~~~~~~

The scope of a variable is the region of code where it is visible (and accessible). There are three types of scope: global scope, module scope
and function scope. When a variable is declared with the keyword ``var``, it is declared as a local variable in the current scope: if it is declared in a function, it is a local variable visible anywhere inside that function (i.e. it has function scope), otherwise it has module scope
and is visible anywhere in the script where it is defined (each script corresponds to a module). If a variable is defined without the ``var`` 
keyword, it is a global variable which is visible everywhere (i.e. it has global scope). Observe the following difference:

.. code:: phon

    var a = 1 # a is a local variable visible in the current module
    b = 2 # b is a global variable visible from other modules


In general, using global variables is strongly discouraged as it can be difficult to reason about them and it is easy to inadvertantly overwrite
a value is there already exists a global variable with the same name. The only place where the use of global variables is recommended is in the
console, as it saves some typing. 


Built-in data types
-------------------


Null
~~~~

The ``Null`` type is a special type that has only one value, namely ``null`` (in lower case). It is used to represent an invalid value.

Boolean
~~~~~~~

A ``Boolean`` can take on two values: ``false`` and ``true``. Boolean values are used to express truth conditions about the state of a program. All conditions in control structures must evaluate to a ``Boolean`` value. There are only four values that are interpreted as false: ``null``, ``false``, ``0`` and ``undefined`` (a special invalid numeric value). All other values are interpreted as true. 


Number
~~~~~~

The ``Number`` type is used to represent real numbers. There are three special values of type ``Number`` that deserve some attention: ``+Infinity`` (positive infinity), ``-Infinity`` (negative infinity) and ``undefined``. The latter value represents an invalid number, such as the logarithm of a negative number. 

Note that the decimal point is always represented by the symbol ``.`` (dot), even if the language of your operating system uses a different symbol (some languages, such as French, use a comma instead). 

String
~~~~~~

A ``String`` represents an ordered sequence of characters, represented as Unicode *code points*. Strings must be enclosed between double quotes or single quotes. Thus, ``"abc"`` and ``'abc'`` represent the same string, which is formed by the concatenation of the three characters ``a``, ``b`` and ``c``. Code points may correspond to single letters, but they can represent more complex units. For example, the string ``"é"`` contains one code point, even though it represents a composite character (the letter ``e`` + an acute accent). Likewise, the string ``"한글"`` (the name of the Korean alphabet, in Korean) contains two code points, although it is composed of two syllables, each of which contains three letters.

Internally, strings are encoded as UTF-8, which is the most widespread Unicode encoding. Source files are also expected to be encoded in UTF-8. 

Strings are immutable, which means that they can never be modified. Functions which "modify" a string always return a modified copy, leaving the
original string unchanged.

You can use the concatenation operator ``&`` to concatenate two or more values. If they are not strings, they will automatically 
 be converted to strings.

.. code:: phon

    var pi = 3.14
    print("The value of pi is " & pi)
    

List
~~~~

A ``List`` is an ordered collection of items. Like strings, lists can be modified and their capacity is automatically adjusted when items are added. Lists can be created directly using a *list literal*:

.. code:: phon

    var lst = [ "a", "b", "c", 3.14 ]


The variable ``lst`` contains four elements, three strings and one number. To access elements in the list, we use array indexing by using the name of the variable followed by square brackets containing the index, as follows:

.. code:: phon
    
    print(lst[2]) # prints "b"


We can also assign a new value at a given index, like so: 

.. code:: phon

    lst[3] = "C"


Indices start at 1 and can be negative: -1 represents the last element, -2 the second-to-last element, and so on.


Array
~~~~~

An ``Array`` is a two-dimensional numeric array (i.e. a matrix). Elements in a matrix are accessed with a set of two indices, noted *(i, j)*,
where *i* represents the *i* th row and *j* represents the *j* th column. Indices in each dimension start at 1 and can be negative.
(Negative indices start from the end of the dimension.)

You can create a new array by passing the number of rows and columns to the constructor. For instance, here is how to create an array containing 3 rows and 4 columns:

.. code:: phon

    var array = new Array(3, 4)
    
    for i = 1 to array.row_count do

        for j = 1 to array.column_count do
            array.set(i, j, i + j)
        end
    end
    print(array)

This code will produce the following output:


.. code:: phon

    2.0000000000, 3.0000000000, 4.0000000000, 5.0000000000, 6.0000000000
    3.0000000000, 4.0000000000, 5.0000000000, 6.0000000000, 7.0000000000
    4.0000000000, 5.0000000000, 6.0000000000, 7.0000000000, 8.0000000000


Object
~~~~~~

An ``Object`` represents an unordered mapping of key/value pairs. Each key/value pair represents a *field*. Keys must be strings and are always unique, whereas values can be anything. Objects can be declared with object literals:

.. code:: phon

    var person = { "name" : "john", "surname" : "smith", "age" : 38 }

In this example, we declare an object with three pairs (separated by commas): the key and value are separated by the : (colon) symbol. This table could correspond to mappings from names (keys) to ages (values) for instance. Note that there is no need for the keys and/or values to be homogeneous: any valid Value (even null!) may appear in an object. Note that even though we declared key/value pairs in a specific order in our example, there is no guarantee that they will be stored in this particular order. For all practical purposes, tables should be regarded as unordered collections, which means that the order of their elements is random. 


When the keys are strings which are valid variable names, we don't need to surround them with quotes. The ``person`` variable could also be declared as follows:

.. code:: phon

    var person = { name : "john", surname : "smith", age : 38 }


There are two ways to access fields in an object. We can use array indexing like for tables, but using a string key instead:

.. code:: phon

    print(person["name"]) # prints "john"

Alternatively, if the key is a valid identifier, we can use the *dot notation*:

.. code:: phon

    person.name = "John"
    print(person.name) # prints "John"




Function
~~~~~~~~

A ``Function`` is a special object which can be used to create reusable blocks of code. Functions are created using the
keyword ``function``. Here is an example of a function which calculates the area of a rectangle.
It expects two arguments (``x`` and ``y``), which correspond to the rectangle's height and width. 

.. code:: phon
    
    function area(x, y)
        return x * y
    end


We can then *call* the function with specific values for ``x`` and ``y``:

.. code:: phon

    var rect = { height: 100, width: 30 }
    var a = area(rect.height, rect.width)
    print("The area of the rectangle is " & a)


If a function is called with fewer arguments than it expects, missing arguments are replaced by ``null``. 
If a function is called with more arguments than it expects, additional arguments are discarded. 

Functions are first class values, which means that they can be assigned to variables, passed as function argument to 
other functions, and used as a return value inside a function. In fact the defition of ``area()`` above could also be written
as follows: 

.. code:: phon

    var area = function(x, y)
        return x * y
    end


In this case, we create an anonymous function object, and assign the result to a variable named ``area``. 


Control flow
------------

If statement
~~~~~~~~~~~~

It is often necessary to execute a code block only if a certain condition is satisfied. This can be achieved with the ``if`` statement

.. code:: phon

    if extension == ".txt" then
        print("This is a text file")
    elsif extension == ".xml" then
        print("This is an XML file")
    else
        print("extension '" & extension & "' not recognized")
    end


This block of code tries to execute the block following the ``if`` branch if its condition is true, otherwise it tries to execute the first
elsif branch (if any), and if all else fails, it executes the ``else`` branch. The ``elsif`` and ``else`` branches are optional, and there 
is no limit on the number of ``elsif`` branches.


While loop
~~~~~~~~~~

The ``while`` loop allows you to execute a block of code while some condition is true. 

.. code:: phon

    var x = 1
    # Print numbers from 1 to 10
    while x <= 10 do
        print(x)
        x++ # increment x
    end 


Repeat loop
~~~~~~~~~~~

The ``repeat`` loop is similar to the ``while`` loop but there are two key differences: the block of code is executed *until* some condition is 
satisfied, and it is executed at least once since it precedes the evaluation of the condition. 

.. code:: phon

    var x = 1
    # Print numbers from 1 to 10
    repeat
        print(x)
        x++ # increment x
    until x > 10 



For loop
~~~~~~~~

The ``for`` loop, as in other programming languages, is used to iterate through a block of instructions, incrementing (or decrementing) a counter at each iteration. The ``for`` loop must always have a ``start`` condition and an ``end`` condition, and may optionally have a ``step`` condition, which indicates by how much the counter should be incremented/decremented (if no ``step`` is specified, the default is 1). Here is the single example, which prints the numbers from 1 to 10 (inclusive):

.. code:: phon

    var i

    for i = 1 to 10 do
        print(x)
    end


To print all the odd digits between 1 and 10, we can use the following loop:

.. code:: phon

    for var i = 1 to 10 step 2 do
        print(x)
    end



To iterate in decreasing order, ``downto`` must be used instead of ``to``:

.. code:: phon

    for i = 10 downto 1 do
        print(x)
    end


Foreach loop
~~~~~~~~~~~~

The ``foreach`` loop is similar to the ``for`` loop, but offers a simpler way to iterate over the values of a list and
the keys of an object.

.. code:: phon

    # Iterate over a list
    var lst = ["a", "b", "c"]

    foreach value in lst do
        print(value)
    end

    # Iterate over an object
    var person =  { name : "john", surname : "smith", age : 38 }

    foreach key in person do
        print(key & " -> " & person[key])
    end


Errors
------

It is sometimes necessary to interrupt a script because it can no longer proceed further. To signal an error, use the ``error()`` 
function, which takes as an argument a string explaining what kind of error happened. For example:

.. code:: phon

    function area(x, y)
        if x < 0 or y < 0 then
            error("x and y must be non-negative")
        end

        return x * y
    end


Another way to trigger errors is to use the ``assert()`` function, which expects a Boolean expression as its first argument, followed by 
an optional error message. It will trigger an error with the error message if the condition is false. 

.. code:: phon

    function area(x, y)
        assert(x >= 0, "x must be non-negative")
        assert(y >= 0, "y must be non-negative")
        return x * y
    end

