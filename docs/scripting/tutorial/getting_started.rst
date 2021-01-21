Getting started
===============


Phonometrica's scripting language was designed to be simple to use yet powerful. It bears many similarities with, and draws inspiration from, 
existing scripting languages such as Python, Lua, MATLAB and R. Familiarity with any of these languages will certainly be helpful, but is not required
and the documentation does not assume that the reader has any experience in programming.


Fundamental notions
-------------------

The 'print' statement
~~~~~~~~

The first thing one is usually taught when one learns a new language is how to display
the text "hello world!". There is no reason for us to break with tradition... The following piece of code illustrates
how to achieve this using the ``print`` statement, which prints text to Phonometrica's console. The statement is preceded by one line of comment. Comments start with 
the symbol ``#`` and end at the end of the line.

.. code:: phon

    # My first script
    print "hello world!"


Comments can also follow statements, so we could also write something like this:


.. code:: phon

    print "hello world!" # My first script


To print several values at once on the same line, you can separate them with commas:

.. code:: phon

    print "h", "e", "l", "l", "o"

By default, Phonometrica will append a new line character at the end. If you don't want that to happen, you can add an extra comma at the end:


.. code:: phon

    print "h", "e", "l", "l", "o",
    print " ",
    print "world!"

The above example is another (rather convoluted) way of printing the line ``"hello world!"``, as in the first example.


**Note about comments**: Although comments can be helpful, we recommend to use them sparingly. In general, a comment should describe *what* the code does or *why*
something non-trivial is done in a certain way, not *how* things are done. 



Variables
~~~~~~~~~

All programming languages allow you to store and refer to values using *variables*. A variable must start with a letter (upper case or lower case)
and can followed by any letter, any digit or the symbol ``_``. Additionally, it can end with the symbol ``$``. The name of variables is case-sensitive, which means that ``myvar`` and ``Myvar`` 
are treated as two different variables. To create a new variable, simply assign a value to it using the assignment operator ``=``: if the variable doesn't exist, it will be created:

.. code:: phon

    x = 5
    print x


In this example, the variable ``x`` is created, since it doesn't exist, and it is simultaneously assigned the value 5 (an ``Integer``). Because Phonometrica's 
scripting language is dynamic, variables can be bound to values of any type. In the following example, ``x`` is first declared as an integer, and is subsequently used to store a string:


.. code:: phon

    x = 5
    print x # prints "5"
    x = "hello"
    print x # prints "hello"



Built-in data types
-------------------


Null
~~~~

The ``Null`` type is a special type that has only one value, namely ``null`` (in lower case). It is used to represent an invalid value.


Boolean
~~~~~~~

A ``Boolean`` can take on two values: ``false`` and ``true``. Boolean values are used to express truth conditions about the state of a program. All conditions in control structures must evaluate to a ``Boolean`` value. There are only four values that are interpreted as false: ``null``, ``false``, ``0`` and ``nan`` (a special invalid numeric value). All other values are interpreted as true. 


Integer
~~~~~~~

An ``Integer`` represents a whole number, which can be positive or negative (e.g. ``0``, ``1``, ``-12``). Internally, integers are represented as an 
integral number whose size is equal to a machine word. This means that on modern 64 bit machines, an integer occupies 64 bits (or 8 bytes) and its 
value can range from -9223372036854775808 to 9223372036854775807. Note that some operators (such as the division operator ``/``) and functions 
will automatically convert an ``Integer`` to a ``Float`` if needed.  


Float
~~~~~

The ``Float`` type is used to represent real numbers, such as ``3.1``, ``-153.9583`` or ``7.0``. Real numbers are represented as double-precision floating point numbers,
which use 64 bits (8 bytes). 

There is a special float called ``nan`` ("not a number"), which represents an invalid numeric value. This is the value that you get when you try to measure pitch in an unvoiced part of the speech signal, for instance.

Note that the decimal point is always represented by the symbol ``.`` (dot), even if the language of your operating system uses a different symbol (some languages, such as French, use a comma instead). 

Number
~~~~~~

``Number`` is an abstract numeric type, which is the base type for ``Integer`` and ``Float``. Some functions specifically request integers or floats as their arguments,
while others accept both; in the latter case, the type of the argument(s) is usually ``Number``, which is compatible with both ``Integer`` and ``Float``.



String
~~~~~~

A ``String`` represents an ordered sequence of characters. Characters are understood as "extended grapheme clusters" in the sense of the Unicode,
specification. Strings must be enclosed between double quotes or single quotes. Thus, ``"abc"`` and ``'abc'`` represent the same string, which is formed by the concatenation of the three characters ``a``, ``b`` and ``c``. 
Characters may correspond to single letters, but they can represent more complex units. For example, the string ``"é"`` is treated 
one character, even though it is composed of the letter ``e`` and an acute accent. Likewise, the string ``"한글"`` (the name of the Korean alphabet, in Korean) contains two characters, although it is composed of two syllables, each of which contains three letters.

Internally, strings are encoded as UTF-8, which is the most widespread Unicode encoding. Source files are also expected to be encoded in UTF-8. 

You can use the concatenation operator ``&`` to concatenate two or more values. If they are not strings, they will automatically 
 be converted to strings, if possible.

.. code:: phon

    pi = 3.14
    print "The value of pi is " & pi
    
Unlike most scripting languages, strings in Phonometrica are *mutable*, which means that some functions can modify them directly:

.. code:: phon

    s = "hello"
    append(s, " world!")
    print s # prints "hello world!"


List
~~~~

A ``List`` is an ordered collection of items. Like strings, lists can be modified and their capacity is automatically adjusted when items are added. Lists can be created directly using a *list literal*:

.. code:: phon

    lst = [ "a", "b", "c", 3.14 ]


The variable ``lst`` contains four elements, three strings and one number. To access elements in the list, we use array indexing by using the name of the variable followed by square brackets containing the index, as follows:

.. code:: phon
    
    print lst[2] # prints "b"


We can also assign a new value at a given index, like so: 

.. code:: phon

    lst[3] = "C"


Indices start at 1 and can be negative: -1 represents the last element, -2 the second-to-last element, and so on.


Array
~~~~~

An ``Array`` is a one or two dimension numeric array. Elements along each dimension start at 1 and can be negative.
(Negative indices start from the end of the dimension.) Two-dimensional arrays are accessed with a pair of indices noted *(i, j)*,
where *i* represents the *i*\ th row and *j* represents the *j*\ th column. To get or set an element in an array, use the index ``[]`` operator. 

You can create a new array by passing the size of each dimension to the constructor. For instance, here is how to create an array containing 3 rows and 4 columns:

.. code:: phon

    array = Array(3, 4)
    
    for i = 1 to array.row_count do
        for j = 1 to array.column_count do
            array[i,j] = i + j
        end
    end

    print array

This code will produce the following output:


.. code:: phon

    @[2.0000000000, 3.0000000000, 4.0000000000, 5.0000000000, 6.0000000000
      3.0000000000, 4.0000000000, 5.0000000000, 6.0000000000, 7.0000000000
      4.0000000000, 5.0000000000, 6.0000000000, 7.0000000000, 8.0000000000]


Another way to produce the same output would be to use an array literal, which is indicated with the ``@[]`` operator. Inside the brackets, rows are separated by commas and 
columns are separated by semicolons. Therefore, our array could be written as follows:

.. code:: phon

    array = @[2, 3, 4, 5, 6; 3, 4, 5, 6, 7; 4, 5, 6, 7, 8]


Table
~~~~~

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


Set
~~~

A ``Set`` represents an ordered collection of unique values. Sets can be declared using a *set literal*:

.. code:: phon

    names = { "john", "peter", "anna", "patricia" }

The declaration of a set is similar to that of a table, except there are only values, no keys. Printing the set in the 
above example will yield the following output:

.. code:: phon

    {"anna", "john", "patricia", "peter"}

As we can see, elements are not ordered according to the way they were declared, but instead appear in lexicographic order. This is because the values
in a set are always *ordered in ascending order*. This means that values in a set must have compatible types and must be comparable.

Sets are useful to keep track of a collection of (unique) values. 


Function
~~~~~~~~

A ``Function`` is a special construct that represents a reusable block of code. Functions are created using the
keyword ``function``. Here is an example of a function that prints the area of a rectangle.
It expects two arguments (``x`` and ``y``), which correspond to the rectangle's height and width. 

.. code:: phon
    
    function area(x, y)
        print "The area of the rectangle is ",  x * y
    end


We can then *call* the function with specific values for ``x`` and ``y``:

.. code:: phon

    area(100, 30) # prints 3000


In addition to executing statements, functions can also send a value back to the caller. This is achieved with the keyword ``return`` 
followed by the expression we want to send back to the caller. The following example illustrates how this can be done. First, we create the 
function ``fibonacci`` to calculate the *n*\ th Fibonacci number. Next, create a list in which we store the first 10 Fibonacci numbers, and 
finally we print the list. 

.. code:: phon

    function fibonacci(num)
        local a = 1
        local b = 0
        local temp

        while num >= 0 do
            temp = a
            a += b
            b = temp
            num -= 1
        end

        return b
    end

    result = []
    for i = 1 to 10 do
        append(result, fibonacci(i))
    end

    print result # prints [1, 2, 3, 5, 8, 13, 21, 34, 55, 89]


Object
~~~~~~

``Object`` is an abstract type: it is the base type for all types in Phonometrica. This means that all types inherit from ``Object``, directly or indirectly. ``Object`` 
is the default :ref:`parameter type <funcparam>` for functions.

Class
~~~~~

A ``Class`` represents a type. Every type has a corresponding class, and every class describes a type. You can get the type of a value with
the function ``type``:

.. code:: phon

    s = "hello"
    i = 10
    f = 10.0

    print type(s) # prints <class String>
    print type(i) # prints <class Integer>
    print type(f) # prints <class Float>


Because classes are also values, you can pass them as arguments to functions, return them from functions, or query their type:


.. code:: phon

    print type(Integer) # prints <class Class>
    print type(Class)   # prints <class Class> 


Module
~~~~~~

A ``Module`` is an object that can be used to store unordered key/value pairs. Each pair represents a *field*. Conceptually, it is similar to a ``Table``, 
except that all its keys must be strings. There are two ways to access fields in a module. We can use array indexing like for tables:

.. code:: phon

    m = Module("My first module")
    m["version"] = "0.1"
    print m["version"]

But we can also use the *dot operator*:

.. code:: phon

    print m.version
    m.greet = function() print "hello" end
    m.greet()     # call module function using the dot operator
    m["greet"]()  # call module function using the index operator


As you can see in the above example, the dot operator and the index operator are equivalent: the dot operator is shorter and more legible, but the index operator
is more flexible since it allows you to create keys dynamically:

.. code:: phon

    keys = ["a", "b", "c"]
    foreach key in keys do
        m[key] = to_upper(key)
    end

    print m.a         # prints "A"
    print m[keys[-1]] # prints "C"

Modules are particularly useful if you intend to redistribute scripts or create plugins. See the :ref:`dedicated page <modules>`.



Control flow
------------

If statement
~~~~~~~~~~~~

It is often necessary to execute a code block only if a certain condition is satisfied. This can be achieved with the ``if`` statement

.. code:: phon

    if extension == ".txt" then
        print "This is a text file"
    elsif extension == ".xml" then
        print "This is an XML file"
    else
        print "extension '", extension, "' not recognized"
    end


This block of code tries to execute the block following the ``if`` branch if its condition is true, otherwise it tries to execute the first
elsif branch (if any), and if all else fails, it executes the ``else`` branch. The ``elsif`` and ``else`` branches are optional, and there 
is no limit on the number of ``elsif`` branches. The ``else`` branch, if it exists, but always come last.


There is a short version of the ``if`` statement which takes the following form:

.. code:: phon

    expression1 if condition else expression2


This expression is called *conditional expression*, and it evaluates to ``expression1`` if ``condition`` is true, and to ``expression2`` otherwise. Consider the
following example: 

.. code:: phon

    x = 7 % 2
    y = "odd" if x == 1 else "even"
    print y

We define ``x`` as the remainder of the division of 7 by 2, which is 1. We then assign the result of the conditional expression that evaluates ``x == 1`` to ``y``. Since 
``x`` is indeed equal to 1, the string that will be printed is ``odd``. 


While loop
~~~~~~~~~~

The ``while`` loop allows you to execute a block of code while some condition is true. 

.. code:: phon

    x = 1
    # Print numbers from 1 to 10
    while x <= 10 do
        print x
        x += 1
    end 

If you need to exit a loop early, use the keyword ``break``:

.. code:: phon

    x = 0
    while true do
        if x > 10 then
            break
        end
        print x
        x += 1
    end

If you only want to break the current iteration of the loop and move to the next iteration, use the keyword ``continue``: 

.. code:: phon

    # Print odd numbers up to 10
    x = 0
    while x < 10 do
        x += 1
        if x % 2 == 0 then
            continue
        end
        print x

    end



Repeat loop
~~~~~~~~~~~

The ``repeat`` loop is similar to the ``while`` loop but there are two key differences: the block of code is executed *until* some condition is 
satisfied, and it is executed at least once since it precedes the evaluation of the condition. 

.. code:: phon

    x = 1
    # Print numbers from 1 to 10
    repeat
        print x
        x += 1
    until x > 10 



For loop
~~~~~~~~

The ``for`` loop, as in other programming languages, is used to iterate through a block of instructions, incrementing (or decrementing) a counter at each iteration. The ``for`` loop must always have a ``start`` condition and an ``end`` condition, and may optionally have a ``step`` condition, which indicates by how much the counter should be incremented/decremented (if no ``step`` is specified, the default is 1). 
Here is a simple example, which prints the numbers from 1 to 10 (inclusive):

.. code:: phon

    for i = 1 to 10 do
        print i
    end


Note that in this case, we didn't need to declare the variable ``i``: Phonometrica will automatically declare it make it local to the ``for`` loop (i.e. it will only be visible inside the ``for`` loop). 


To print all the odd digits between 1 and 10, we can use the following loop:

.. code:: phon

    for i = 1 to 10 step 2 do
        print i
    end



To iterate in decreasing order, ``downto`` must be used instead of ``to``:

.. code:: phon

    for i = 10 downto 1 do
        print x
    end

You can also use a ``step`` with ``downto``:


.. code:: phon

    for i = 10 downto 1 step 2 do
        print i
    end



Foreach loop
~~~~~~~~~~~~

The ``foreach`` loop is similar to the ``for`` loop, but offers a simpler way to iterate over the content of an iterable object. 

.. code:: phon

    # Iterate over a list
    lst = ["a", "b", "c"]

    foreach value in lst do
        print value
    end


If there is a single loop variable (``value`` in this example), Phonometrica will iterate over the values in the collection. You can add another loop variable 
if you would like to iterate over the indexes (or keys) as well as the values:

.. code:: phon

    # Iterate over a list
    lst = ["a", "b", "c"]

    foreach i, value in lst do
        print i, " -> ", value
    end


Here is another example where we iterate over the keys and values in a table:

.. code:: phon

    person =  { "name" : "John", "surname" : "Smith", "age" : 38 }

    foreach key, value in person do
        print key, " -> ", value
    end


As for the ``for`` loop, the loop variable(s) is/are automatically declared and are made local to the loop.



Here are the builtin types that support iteration with the ``foreach`` loop:

.. list-table:: 
    :widths: 25 25 50
    :header-rows: 1

    * - Type
      - key (optional)
      - value
    * - File
      - line number
      - line 
    * - List
      - index
      - value
    * - Regex
      - index
      - capture  
    * - Set
      - index
      - value  
    * - String
      - index
      - character  
    * - Table
      - key
      - value




Scope of variables
~~~~~~~~~~~~~~~~~~

The scope of a variable is the region of code where it is visible (and accessible). There are three types of scope in Phonometrica: *global*, *local* and *non-local*. 

By default, variables are global: they are visible everywhere. Local variables, on the other hand, are only visible within the block in which they are declared, from the point of
declaration until the end of the block. To declare a variable as local, add the keyword ``local`` before the first assignment to this variable. Consider the following example:

.. code:: phon

    x = "global"

    # Create a new scope
    do
        print x # prints "global"
        local x = "local"
        print x # prints "local"
    end

    print x # prints "global"


As you can see, the ``do ... end``` block creates a new scope: the ``x`` variable declared in this block temporarily hides the global variable with the same name. After the ``do ... end`` block ends,
the global variable becomes visible again. 

Any new block created by an ``if`` statement, a ``for`` loop, a function, etc. defines a new scope. Such scoping rules are sometimes refered to as *lexical scoping*.


Global variables live for as long as Phonometrica is running once they have been defined. In order to avoid "polluting" the global 
namespace, you should use the keyword ``local`` before the declaration of the variable:

.. code:: phon

    local x = "some value"


A top-level variable declared in this way will no longer be visible after the script has been executed. You can also define local functions, which will only be available in the current scope and all embedded scopes:


.. code:: phon

    local function test()
        print "this is a local function"
    end


If you intend to redistribute a script or plugin, we strongly encourage you to declare all variables as local, unless you need them to be global, of course.

Global and local variables are the two most common types of variables, but there is a third type: non-local variables. Consider the following example:

.. code:: phon

    function outer()
        local s = "hello"
        function inner()
            return s
        end

        return inner
    end

    local f = outer()
    print f() # prints "hello"


From the point of view of function ``outer``, the variable ``s`` is local since it is defined in the scope created by that function. But what about function ``inner``? This function creates 
a new scope embedded in ``outer``'s scope, so from ``inner``'s perspective, ``s`` is neither local, since it is not defined in the function's own body, nor global, since it is not visible outside of ``outer``'s scope. 
What is it, then? In this case, ``s`` is regarded as a *non-local* variable in the scope defined by ``inner``. When we declare the variable ``f``, we execute the function ``outer``, 
which first creates a variable named ``s`` and then creates a function named ``inner``, which  *captures* ``outer``'s local variable ``s``. Finally, ``outer`` returns the function ``inner``. 
This means that ``f`` is now a function (the function ``inner``). When we call it, it returns the value of the variable ``s``. Functions that capture non-local variables are called :ref:`closures <closures>`.



Errors
------

It is sometimes necessary to interrupt a script because it can no longer proceed further. To signal an error, use the keyword ``throw`` 
followed by an error message. Here is an example:

.. code:: phon

    function area(x, y)
        if x <= 0 or y <= 0 then
            throw "x and y must be positive"
        end

        return x * y
    end


Assertions
----------


Another way to trigger errors is to use the keyword ``assert`` followed by a Boolean expression that must be true, and an optional error 
message. It will trigger an error with the error message if the condition is false. 

.. code:: phon

    function area(x, y)
        assert x > 0, "x must be positive"
        assert y > 0, "y must be positive"
        return x * y
    end


Debugging
---------

It is sometimes necessary to include debugging information to check the state of the program at any given point. A common way to achieve this
is to include ``print`` statements, to comment them out once the program has been debugged, and to uncomment them if we need to debug the 
program again. This approach is fine for small programs, but it can be tedious and unreliable for larger programs. 

Phononometrica's scripting language offers a nicer alternative: you can use the keyword ``debug`` followed by a statement. The statement will only
be executed in debug mode:

.. code:: phon

    function area(x, y)
        debug assert x > 0
        debug assert y > 0
        return x * y
    end


Alternatively, you can create ``debug`` blocks, which can be more convenient if you have a lot of debugging code:

.. code:: phon

    function area(x, y)
        debug 
            assert x > 0
            assert y > 0
            # The following line will only be printed if both assertions succeed.
            print "x and y are both positive"
        end
        return x * y
    end


You can control wether debuggin is on (default) or off using an ``option`` statement, which must be at the beginning of your script 
before any other statements. It can one of the following three forms:

.. code:: phon

    option debug           # turns debugging on (which is the default)
    option debug = true    # turns debugging on (equivalent to the line above, but more explicit)
    option debug = false   # turns debugging off



Operators
---------

Mathematical operators
~~~~~~~~~~~~~~~~~~~~~~

Phonometrica supports the following mathematical operators: ``+`` (addition), ``-`` (subtraction), 
``*`` (multiplication) and ``/`` (division), ``^`` (power) and ``%`` (modulus). The power operator has highest precedence,
followed by the multiplication, division and modulus operators. Addition and subtraction have lowest precedence. You can use 
grouping parentheses ``()`` to alter the precedence of operators:

.. code:: phon

    print 3 + 5 * 10   # prints 53
    print (3 + 5) * 10 # prints 80


Boolean operators
~~~~~~~~~~~~~~~~~

Phonometrica supports the 3 standard Boolean operators ``and``, ``or`` and ``not``. ``and`` and ``or`` are binary operators: ``x and y`` is 
true if both ``x`` and ``y`` are true, whereas ``x or y`` if ``x`` is true or ``y`` is true (or both are true). ``not`` is a unary operator:
``not x`` is true if ``x`` is false, and vice versa. 

Note that in the case of ``and`` and ``or``, Phonometrica will not necessary evaluate the second operand. For instance, in the expression
``x and y``, ``y`` will not be evaluated if ``x`` is false, since ``x and y`` will always be false whatever the truth condition of ``y`` is; 
likewise, ``y`` will not be evaluated in ``x or y`` if ``x`` is true since this is enough to determine that the whole expression is true.
Therefore, you shouldn't rely on the second operand being evaluated. 

Comparison operators
~~~~~~~~~~~~~~~~~~~~

Like most programming languages, Phonometrica's scripting language allows you to use a number of binary operators that compare their operands:

- ``x == y`` is true if ``x`` is equal to ``y``
- ``x != y`` is true if ``x`` is not equal to ``y``
- ``x < y`` is true if ``x`` is less than ``y``
- ``x <= y`` is true if ``x`` is less than or equal to ``y``
- ``x > y`` is true if ``x`` is greater than ``y``
- ``x >= y`` is true if ``x`` is greater than or equal to ``y``

In addtion, the operator ``<=>`` (sometimes called the "spaceship operator") can be used to compare values. The expression 
``x <=> y`` evaluates to:

- ``-1`` if ``x`` is less than ``y``
- ``0`` if ``x`` is equal to ``y``
- ``1`` if ``x`` is greater than ``y``


Concatenation operator
~~~~~~~~~~~~~~~~~~~~~~~~

The concatenation operator ``&`` allows to concatenate two or more strings. It implicitly converts values to
``String`` if needed:

.. code:: phon

    pi = 3.14
    s = "The value of pi is" & pi
    print s
