Functions
=========

Phonometrica's scripting language is an object-oriented programming language.  Unlike most other object-oriented programming languages,
however, Phonometrica is based on the notion of *multiple dispatch*: when a function is called and there are several functions with the 
same name, Phonometrica will decide which function to call based on the type of its arguments. This page explains what multiple dispatch
is and how to use functions efficiently in Phonometrica.


Basics
------


A ``Function`` is a special construct that represents a reusable block of code. Functions are created using the
keyword ``function``. Here is an example of a function that prints the area of a rectangle.
It has two *parameters* (``x`` and ``y``), which correspond to the rectangle's height and width. 

.. code:: phon
    
    function area(x, y)
        print "The area of the rectangle is ",  x * y
    end


We can then *call* the function with specific values (called *arguments*) for ``x`` and ``y``, using parentheses after the name of the function and passing the arguments to the 
function by putting them inside the parentheses:

.. code:: phon

    area(100, 30) # prints 3000


In addition to executing statements, functions can also send a value back to the caller. This is achieved with the keyword ``return`` 
followed by the expression we want to send back to the caller. Let's rewrite the above code in a slightly different way:


.. code:: phon

    function area(x, y)
        return x * y
    end

    a = area(100, 30)
    print "The area of the rectangle is ", a


In this new example, the function ``area`` is only responsible for computing the area and returning the value. All the printing is done 
outside the function. 


Note: Functions are first class values in Phonometrica, which means that they can be assigned to variables, passed as function arguments to 
other functions, and even used as a return value inside a function. 

.. _funcparam:

Function parameters
-------------------

Our function ``area`` takes 2 parameters, ``x`` and ``y``, which we expect to be numbers. But what happens if we inadvertantly pass a value
that has a different type?


.. code:: phon

    function area(x, y)
        return x * y
    end

    var a = area(100, "30")


Phonometrica will throw an error because it can't apply the math operator ``*`` to a number and a string. But we might not always 
be that lucky, and we might accidently introduce subtle and hard-to-find bugs if we pass the wrong type of argument and Phonometrica proceeds
with it without detecting that there is a problem. 

Fortunately, Phonometrica allows us to minimize this kind of problem by specifying a type for each parameter. If we don't specify a type for a parameter, 
Phonometrica will implicitly assign it the type ``Object``, which is the base type for all Phonometrica types. Our function could have 
equivalently been written as follows:

.. code:: phon

    function area(x as Object, y as Object)
        return x * y
    end

The two forms are strictly equivalent: the former is shorter to type, the latter is more explicit. When the type of a parameter is ``Object``,
any value can be passed because all types inherit from Object, directly or indirectly. To make our code more robust, we could limit the 
types of the parameters to numbers. This is done as follows:

.. code:: phon

    function area(x as Number, y as Number)
        return x * y
    end


If we now try to call the function with a number and a string:

.. code:: phon

    a = area(100, "30")


Phonometrica will not even try to execute the function; it will give us a clear error message:

.. code::

    Line 5: Cannot resolve call to function 'area' with the following argument types: (Integer, String).
    Candidates are:
    area(Number, Number)


Type information is optional: if a parameter can accept any value, you can simply omit the type (or declare the type as ``Object`` to make your 
intent clearer). Omitting type information can also save you some typing for small scripts. For scripts that you intend to redistribute, 
however, we strongly encourage you to add type information because it will make your code more robust and will clarify the 
intended use of your functions.


Function overloading
--------------------

Suppose that we want to create a function to concatenate two values. We want it to work with either two strings or two lists. One approach 
would be to create a function that accepts two objects, and then decides what to do depending on the type of the objects:

.. code:: phon

    function concat(x, y)
        if type(x) == String and type(y) == String then
            return x & y
        elsif type(x) == List and type(y) == List then
            local result = []
            foreach v in x do
                append(result, v)
            end
            foreach v in y do
                append(result, v)
            end

            return result
        else
            throw "Invalid types in concat()"
        end
    end


This approach works, but it is really tedious. Phonometrica offers a cleaner and more robust alternative: *function overloading*. Phonometrica
lets you create functions with the same name, in the same scope, as long as they have a different number of parameters and/or the type of the parameters
are different. We can thus rewrite our big function as two smaller functions:

.. code:: phon

    function concat(x as String, y as String)
        return x & y
    end

    function concat(x as List, y as List)
        local result = []
        foreach v in x do
            append(result, v)
        end
        foreach v in y do
            append(result, v)
        end

        return result
    end

We no longer need to take care of the error case ourselves, because Phonometrica will do it automatically for us. For example, if we try to
call ``concat`` with two integers:

.. code:: phon

    concat(3, 5)

We will get the following error:

.. code::

    Line 17: Cannot resolve call to function 'concat' with the following argument types: (Integer, Integer).
    Candidates are:
    concat(String, String)
    concat(List, List)




To find out which function it should call, Phonometrica considers all functions that have the same name and number of pareameters as in the 
function call, and calculates for each of them a *cost* based on the type of the corresponding argument in the call. The cost of a function is calculated as the sum of the inheritance distances between each argument's type and the expected parameter type. For instance, if a function's parameter's type 
is ``Object`` and the function is called with a ``Float`` as an argument, the cost will be 2 because ``Float`` inherits from ``Number``, which 
inherits from ``Object``. If an argument doesn't inherit from the corresponding function parameter, the function is discarded as a potential
candidate. The function that is called is the one with the lowest cost.

There are two special cases to be aware of. First, when an argument is ``null``, Phonometrica will assign a cost of 0 for this argument no 
matter what the type of the parameter is. This makes it possible to pass null values as functions to signal that the value is invalid. 
Secondly, it is sometimes the case that two or more functions are equally good candidates. Consider the following example:

.. code:: phon

    function test(x as Integer, y as Number)
        pass
    end

    function test(x as Number, y as Integer)
        pass
    end

    test(1, 2)

This chunk of code produces the following error, because the call is ambiguous:


.. code::

    Line 9: [Runtime error] Cannot resolve ambiguity in call to function 'test' with the following argument types: (Integer, Integer).
    Candidates are:
    test(Integer, Number)
    test(Number, Integer)


To understand why this call is ambiguous, let's calculate the cost for each overload. In the first function, we pass an ``Integer`` as the first argument and expect an
``Integer``, so the cost for ``x`` is 0, and the second argument is an ``Integer`` and we expect a ``Number``, so the cost is 1. The cost 
for this function is therfore 0 + 1 = 1. Following the same reasoning, the cost for the second overload would be 1 + 0 = 1. Since both functions
have the same cost and there is no function with a lower cost, Phonometrica throws an error. To solve this problem, we would need to either 
modify one of the overloads, or add a new one that is more specific. Here, we could simply add a third overload:


.. code:: phon

    function test(x as Integer, y as Integer)
        print "Now this works!"
    end



Value and reference parameters
------------------------------


:ref:`Clonable types <clonability>` in Phonometrica have value semantics, which means that assigning a variable to another one copies its 
value. Value semantics extends to function parameters: by default, function parameters are *passed by value*. Suppose we want to create 
a function that appends an element to a list but ensures that the element is not ``null``. We could write it like that:

.. code:: phon

    function append_item(list as List, item as Object)
        if item == null then
            throw "Cannot append a null item"
        end
        append(list, item)
    end


However, if we try to use it, it will not work as expected:

.. code:: phon

    lst = [1, 2, 3, 4]
    append_item(lst, 5)
    print lst # prints [1, 2, 3, 4]


Since the ``List`` type has value semantics, a copy of ``lst`` will be passed to ``append_item``, and this copy (``list``) will be modified 
but the original value will be unaffected. For our function to be able to work as intended, we need the first argument to be *passed by reference*. 
This is achieved by adding the keyword ``ref`` before the corresponding parameter:

.. code:: phon

    function append_item(ref list as List, item as Object)
        if item == null then
            throw "Cannot append a null item"
        end
        append(list, item)
    end

    var lst = [1, 2, 3, 4]
    append_item(lst, 5)
    print lst # prints [1, 2, 3, 4, 5]



.. _closures:



Closures
--------


Functions can be defined inside other functions. Such nested functions have access to their enclosing scope(s): as a result, they can *capture* variables in their environment (*non-local* variables) and 
keep a reference to them, even if they go out of scope. Such functions are called *closures*. Consider the following example:

.. code:: phon

    function make_counter()
        local x = 0
        function inner()
            x += 1
            return x
        end

        return inner
    end

    counter1 = make_counter()
    counter2 = make_counter()
    print counter1() # prints 1
    print counter1() # prints 2
    print counter1() # prints 3
    print counter2() # prints 1


Let's go through the above code chunk to understand what it does. When we create ``counter1``, we execute the function ``make_counter``, which first creates a variable named ``x`` and then creates a function named ``inner``, which 
*captures* ``make_counter``'s local variable ``x``. Finally, ``make_counter`` returns the function ``inner``. This means that ``counter1`` is now a function (the function ``inner``). When we initialize ``counter2``, we call ``make_counter`` again: it will create a new variable 
named ``x`` and a new function named ``inner``, which it will return. As a result, ``counter1`` and ``counter2`` each have their own "version" of ``inner`` and ``x``. Each time a counter is called, 
it will call its own version of ``inner``, which will increment its own version of ``x``. Functions which can capture non-local variables, such as ``inner`` in this example, are called *closures*. 

Closures are a powerful construct that allows us to create *stateful* functions, that is functions that can retain state across calls. In the above example, the state is the counter represented by
the variable ``x``. In the above example, a closure was used to create a *generator*, i.e. a function that generates a new value every time it is called, depending on its internal state. 
Here is another example of a closure which generates the next number in the Fibonacci sequence every time it is called. 

.. code:: phon

    function fibonacci()
        local first = 0
        local second = 0

        function fib()
            if first == 0 then
                first = 1
                second = 1
                return 0
            else
                local current = first   
                local tmp = second
                second = first + second
                first = tmp

                return current
            end        
        end

        return fib
    end

    var f = fibonacci()

    for i = 1 to 10 do
        print f()
    end


Function expressions
--------------------

Another way to use functions is to create a *function expression*. Function expressions are anonymous functions which can be used like 
any other expression. As an example, the following function:

.. code:: phon

    function area(x as Number, y as Number)
        return x * y
    end

could be written equivalently as:

.. code:: phon

    area = function(x as Number, y as Number)
        return x * y
    end

The advantage of function expressions is that you can use them wherever you can use an expression, for instance as the return value of another
function:

.. code:: phon

    function make_counter(start as Integer)
        return function()
            local n = start
            start += 1
            return n
        end
    end

    counter = make_counter(10)
    print counter() # prints 10
    print counter() # prints 11

    
As you can see, in this example, we create a closure that captures the non-local variable ``start``, but this closure is an anonymous function expression, 
which we can return directly. 

Note: created a named function and and assigning a function expression to a variable are strictly equivalent in the top-level scope, but they are slightly different
when they are created in an embedded scope: variables are always global, unless they are declared with the keyword ``local``, whereas functions are local to the scope, 
whether they are declared as local or not. Consider the following example:


.. code:: phon
    # This function is global
    function outer1()
        print "I'm a global function"

        # This function is local
        function inner1()
            print "I'm a local function only visible in outer1"
        end

        inner1()
    end

    # This function is local to the top-level scope (it won't be visible after the script has run)
    local function outer2()
        print "I'm a local function in the top level scope()"

        # The keyword 'local' is unnecessary, the behaviour is the same as inner1
        local function inner2()
            print "I'm a local function only visible in outer2"
        end
    end


Implicit return values
----------------------

As we saw above, we can explicitly return a value from a function using the keyword ``return``. In addition, there are two scenarios in which 
Phonometrica will implicitly return a value. First, all functions that don't explicitly return a value implicitly return the value ``null``.
For instance, the following piece of code is valid:

.. code:: phon

    function do_nothing()
        pass
    end

    x = do_nothing()
    assert x == null


Secondly, if you compute an expression and don't assign its result, Phonometrica will use it as the function's return value. (If you compute
several expressions in the function, Phonometrica will use the result of the last one.) The two functions below are equivalent:

.. code:: phon

    function test1()
        return 3
    end

    function test2()
        3
    end

    assert test1() == test2()

Implicit return values can be used in function expressions: this offers a compact way to create short anonymous functions:

.. code:: phon

    function modify(ref strings as List, f as Function)
        foreach i, ref s in strings do
            s = f(s)
        end
    end

    names = ["toto", "tata", "titi"]
    modify(names, function(x) x & ".txt" end) 
    print names # prints ["toto.txt", "tata.txt", "titi.txt"]


