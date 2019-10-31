Statistical functions
=====================

This page describes the statistical functions that are available in Phonometrica.


Global functions
----------------

.. function:: chi2_test(X)

Computes Pearson's chi-squared test on ``X``, which must be a two-dimensional array. The *m* rows in the array represent
the *m* levels of a categorical variable, and the *n* columns represent the *n* levels of another categorical variable.
Each cell represents the unnormalized frequency count for the combination of the two variables. This test evaluates the
null hypothesis that the two variables are independent.

This function returns an object with the following fields:

* ``chi2``: the chi-squared value
* ``df``: the number of degrees of freedom
* ``p``: the p-value

See also: :func:`report_chi2`

------------

.. function:: f_test(x, y [, alternative])

Computes the F-test on ``x`` and ``y`` which must be one-dimensional arrays. This test evaluates the null hypothesis that samples
``x`` and ``y`` have the same variance.

If ``alternative`` is specified, it must be one of the following strings: ``"two-tailed"`` performs a two-tailed test (default), ``"less"`` performs a lef-tailed
test and ``"greater"`` performs a right-tailed test.
This function returns an object with the following fields:

* ``f``: the *F* statistic, which is the ratio between the variance of ``x`` and the variance of ``y``
* ``df``: the number of degrees of freedom
* ``p``: the p-value

------------

.. function:: mean(x [, dim])

Returns the mean of the array ``x``. If ``dim`` is specified, returns an ``Array`` in which each element
represents the mean over the given dimension in a two dimension array. If dim is equal to 1, the calculation is performed
over rows. If it is equal to 2, it is performed over columns.

------------

.. function:: report_chi2(X)

Computes and reports Pearson's chi-squared test on ``X``, which must be a two-dimensional array. This is a convenience wrapper
over ``chi2_test()``.

See also: :func:`chi2_test`

------------

.. function:: std(x [, dim])

Returns the standard deviation of the array ``x``. If ``dim`` is specified, returns an ``Array`` in which each element
represents the standard deviation over the given dimension in a two dimension array. If dim is equal to 1, the calculation is performed
over rows. If it is equal to 2, it is performed over columns.

See also: :func:`vrc`, :func:`mean`

------------

.. function:: sum(x [, dim])

Returns the sum of the elements in the array ``x``. If ``dim`` is specified, returns an ``Array`` in which each element
represents the sum over the given dimension in a two dimension array. If dim is equal to 1, the summation is performed
over rows. If it is equal to 2, summation is performed over columns.

------------

.. function:: t_test(x, y [, equal_variance, [, alternative]])

Computes a two-sample independent t-test for the mean between the samples ``x`` and ``y``, which must be one-dimensional
arrays. This test evaluates the null hypothesis that samples ``x`` and ``y`` have equal means.

If ``equal_variance`` is true, the variance of the two samples is assumed to be equal and Student's t-test is calculated,
using the pooled standard error. If ``equal_variance`` is false (default), Welch's t-test is used instead.

If ``alternative`` is specified, it must be one of the following strings: ``"two-tailed"`` performs a two-tailed test (default),
``"less"`` performs a lef-tailed test and ``"greater"`` performs a right-tailed test.
This function returns an object with the following fields:

* ``t``: the *t* statistic
* ``df1``: the number of degrees of freedom of ``x``
* ``df2``: the number of degrees of freedom of ``y``
* ``p``: the p-value


See also: :func:`t_test1`

------------

.. function:: t_test1(x, mu [, alternative])

Computes a one-sample t-test for the sample ``x``, which must a one-dimensional array. This test evaluates the null
 hypothesis that the mean of sample ``x`` is equal to the theoretical mean ``mu``.

If ``alternative`` is specified, it must be one of the following strings: ``"two-tailed"`` performs a two-tailed test (default),
``"less"`` performs a lef-tailed test and ``"greater"`` performs a right-tailed test.
This function returns an object with the following fields:

* ``t``: the *t* statistic
* ``df``: the number of degrees of freedom
* ``p``: the p-value

See also: :func:`t_test`

------------

.. function:: vrc(x [, dim])

Returns the sample variance of the array ``x``. If ``dim`` is specified, returns an ``Array`` in which each element
represents the variance over the given dimension in a two dimension array. If dim is equal to 1, the calculation is performed
over rows. If it is equal to 2, it is performed over columns.

See also: :func:`std`

------------