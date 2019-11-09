Mathematical functions and constants
====================================


This page describes the mathematical functions and constants that are available in Phonometrica. Functions that 
usually accept a ``Number``, such as ``log()``, can also work with arrays, in which case the operation is applied
to each element in the array.


Global functions
----------------

.. function:: abs(x)

Returns the absolute value of ``x``. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.

------------

.. function:: acos(x)

Returns the arccosine of ``x``. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.


------------

.. function:: asin(x)

Returns the arcsine of ``x``. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.


------------

.. function:: atan(x)

Returns the arctangent of ``x``. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.

------------

.. function:: atan2(y, x)

Returns the four-quadrant inverse tangent of ``y`` and ``x``. 

------------

.. function:: ceil(x)

Returns the smallest integer no smaller than ``x``. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.

------------

.. function:: cos(x)

Returns the cosine of ``x``. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.

------------

.. function:: exp(x)

Returns the exponential of ``x``. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.

------------

.. function:: floor(x)

Returns the largest integer that is no larger than ``x``. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.

------------

.. function:: log(x)

Returns the natural logarithm of ``x``. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.

------------

.. function:: log2(x)

Returns the logarithm of ``x`` in base 2. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.

------------

.. function:: log10(x)

Returns the logarithm of ``x`` in base 10. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.

------------

.. function:: max(x, y [, ...])

Returns the maximum value among the arguments passed to the function.

------------

.. function:: min(x, y [, ...])

Returns the minimum value among the arguments passed to the function.

------------

.. function:: pow(x, y)

Returns ``x`` raised to the power of ``y``.

------------

.. function:: random()

Returns a pseudo-random value in the interval [0, 1[ according to a uniform distribution.

------------

.. function:: round(x)

Rounds ``x`` to the nearest integer. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.

------------

.. function:: sin(x)

Returns the sine of ``x``. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.

------------

.. function:: sqrt(x)

Returns the square root of ``x``. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.

------------

.. function:: tan(x)

Returns the tangent of ``x``. If ``x`` is an array, the function returns a copy of the array in which the
function has been applied to each element.



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
