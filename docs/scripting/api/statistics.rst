Statistical functions
=====================

This page describes the statistical functions that are available in Phonometrica.


Global functions
----------------


.. function:: sum(x [,dim])

Returns the sum of the elements in the array ``x``. If ``dim`` is specified, returns an ``Array`` where each element
represents the sum over the given dimension in a two dimension array. If dim is equal to 1, summation is performed
over rows. If it is equal to 2, summation is performed over columns.

------------