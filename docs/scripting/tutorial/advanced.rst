Advanced notions
================


Closures
--------

Functions can be defined inside other functions. Such nested functions have access to their enclosing scope(s): as a result,
they can *capture* variables in their environment and keep a reference to them, even if they go out of scope. Such functions
are called *closures*. Consider the following example:

.. code:: phon

	function outer(c)
	   var b = 1

	   function inner()
	        
	         var a = 2
	         print(a + b + c)
	    end

	   # Return a closure which captures b and c
	   return inner
	end

	# Create two closures
	var f1 = outer(3)
	var f2 = outer(5)


The function ``inner()`` is defined inside ``outer()``, and has access to ``outer``'s variables (its argument ``c`` and its local variable ``b``). The variables ``f1`` and  ``f2`` instances of the function ``inner``, but each of them with a different captured environment. In ``f1``, the 
value of ``c`` is 3, whereas it is ``5`` in ``f2``. Since ``f1`` and ``f2`` are functions, we can call them like any regular function:

.. code:: phon

	f1() # prints "6"
	f2() # prints "8" 


Closures are a powerful construct which allows us to create functions with internal state. They are commonly used to create *generators*, i.e. functions which can generate new values every time they are called, depending on their internal state. Here is an example of a closure which generates the Fibonacci sequence. We use it to print the first ten values in the sequence.

.. code:: phon

	function fibonacci()
	    var first = 0
	    var second = 0

	    function fib()
	        if first == 0 then
	            first = 1
	            second = 1
	            return 0
	        else
	            var current = first   
	            var tmp = second
	            second = first + second
	            first = tmp

	            return current
	        end        
	    end

	    return fib
	end

	var f = fibonacci()

	for var i = 1 to 10 do
	    print(f())
	end