Mathematical functions and constants
====================================


This page describes the mathematical functions and constants that are available in Phonometrica. Functions that 
usually accept a ``Number``, such as ``log()``, can also work with arrays, in which case the operation is applied
to each element in the array.


Global functions
----------------

.. function:: abs(x as Number)

Returns the absolute value of ``x``. 


------------

.. function:: abs(x as Array)

Returns a copy of the array in which ``abs`` has been applied to each element. 


------------

.. function:: acos(x as Number)

Returns the arccosine of ``x``. 


------------

.. function:: acos(x as Array)

Returns a copy of the array in which ``acos`` has been applied to each element. 


------------

.. function:: asin(x as Number)

Returns the arcsine of ``x``. 

------------

.. function:: asin(x as Array)

Returns a copy of the array in which ``asin`` has been applied to each element. 

------------

.. function:: atan(x as Number)

Returns the arctangent of ``x``. 

------------

.. function:: atan(x as Array)

Returns a copy of the array in which ``atn`` has been applied to each element. 



------------

.. function:: atan2(y as Number, x as Number)

Returns the four-quadrant inverse tangent of ``y`` and ``x``. 

------------

.. function:: ceil(x as Number)

Returns the smallest integer no smaller than ``x``.

------------

.. function:: ceil(x as Array)

Returns a copy of the array in which ``ceil`` has been applied to each element. 

------------

.. function:: cos(x as Number)

Returns the cosine of ``x``. 

------------

.. function:: cos(x as Array)

Returns a copy of the array in which ``cos`` has been applied to each element. 

------------

.. function:: exp(x as Number)

Returns the exponential of ``x``. 

------------

.. function:: exp(x as Array)

Returns a copy of the array in which ``exp`` has been applied to each element. 

------------

.. function:: floor(x as Nubmer)

Returns the largest integer that is no larger than ``x``. 

------------

.. function:: floor(x as Array)

Returns a copy of the array in which ``floor`` has been applied to each element. 

------------

.. function:: log(x as Number)

Returns the natural logarithm of ``x``.

------------

.. function:: log(x as Array)

Returns a copy of the array in which ``log`` has been applied to each element. 


------------

.. function:: log2(x as Number)

Returns the logarithm of ``x`` in base 2. 

------------

.. function:: log2(x as Array)

Returns a copy of the array in which ``log2`` has been applied to each element. 

------------

.. function:: log10(x as Number)

Returns the logarithm of ``x`` in base 10. 

------------

.. function:: log10(x as Array)

Returns a copy of the array in which ``log10`` has been applied to each element. 

------------

.. function:: max(x as Number, y as Number)

Returns the larger value between ``x`` and ``y``.

------------

.. function:: max(x as Integer, y as Integer)

Returns the larger value between ``x`` and ``y``.


------------

.. function:: min(x as Number, y as Number)

Returns the smaller value between ``x`` and ``y``.

------------

.. function:: min(x as Integer, y as Integer)

Returns the smaller value between ``x`` and ``y``.

------------

.. function:: random()

Returns a pseudo-random value in the interval [0, 1[ according to a uniform distribution.

------------

.. function:: round(x as Number)

Rounds ``x`` to the nearest integer. 

------------

.. function:: round(x as Array)

Returns a copy of the array in which ``round`` has been applied to each element. 


------------

.. function:: sin(x as Number)

Returns the sine of ``x``. 

------------

.. function:: sin(x as Array)

Returns a copy of the array in which ``sin`` has been applied to each element. 

------------

.. function:: sqrt(x as Number)

Returns the square root of ``x``. 

------------

.. function:: sqrt(x as Array)

Returns a copy of the array in which ``sqrt`` has been applied to each element. 

------------

.. function:: tan(x as Number)

Returns the tangent of ``x``. 

------------

.. function:: tan(x as Array)

Returns a copy of the array in which ``tan`` has been applied to each element. 



Constants
---------


.. attribute:: E

Returns the value of *e*, the base of the natural logarithm (approximately 2.718281).

------------

.. attribute:: PHI

Returns the value of the golden ratio :math:`\phi` (approximately 1.618033).

------------

.. attribute:: PI

Returns the value of pi (approximately 3.141593).

------------

.. attribute:: SQRT2

Returns the square root of 2 (approximately 1.414214).
