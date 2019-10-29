Statistical functions
=====================

This page describes the statistical functions that are available in Phonometrica.


Global functions
----------------

.. function:: chi2_test(X)

Computes Pearson's chi-squared test on ``X``, which must be a two-dimensional array. This function returns an object
with the following fields:

* ``chi2``: the chi-squared value
* ``df``: the number of degrees of freedom
* ``p``: the p-value

------------

.. function:: mean(x [, dim])

Returns the mean of the array ``x``. If ``dim`` is specified, returns an ``Array`` where each element
represents the mean over the given dimension in a two dimension array. If dim is equal to 1, the calculation is performed
over rows. If it is equal to 2, it is performed over columns.

------------

.. function:: report_chi2(X)

Computes and reports Pearson's chi-squared test on ``X``, which must be a two-dimensional array. This is a convenience wrapper
over ``chi2_test()``.

------------

.. function:: std(x [, dim])

Returns the standard deviation of the array ``x``. If ``dim`` is specified, returns an ``Array`` where each element
represents the standard deviation over the given dimension in a two dimension array. If dim is equal to 1, the calculation is performed
over rows. If it is equal to 2, it is performed over columns.

------------

.. function:: sum(x [, dim])

Returns the sum of the elements in the array ``x``. If ``dim`` is specified, returns an ``Array`` where each element
represents the sum over the given dimension in a two dimension array. If dim is equal to 1, the summation is performed
over rows. If it is equal to 2, summation is performed over columns.

------------

.. function:: var(x [, dim])

Returns the sample variance of the array ``x``. If ``dim`` is specified, returns an ``Array`` where each element
represents the variance over the given dimension in a two dimension array. If dim is equal to 1, the calculation is performed
over rows. If it is equal to 2, it is performed over columns.

------------