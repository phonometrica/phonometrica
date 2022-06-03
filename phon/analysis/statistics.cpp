/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne <jeychenne@gmail.com>                                                       *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 28/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cmath>
#include <boost/math/distributions/students_t.hpp>
#include <boost/math/distributions/fisher_f.hpp>
#include <boost/math/distributions/chi_squared.hpp>
#include <phon/analysis/statistics.hpp>

namespace phonometrica { namespace stats {

using boost::math::cdf;
using boost::math::complement;


double sum(const Array<double> &x)
{
    auto count = x.size();
    auto data = x.data();
    double sum = 0;

    for (intptr_t i = 0; i < count; ++i)
    {
        sum += data[i];
    }

    return sum;
}

Array<double> sum(const Array<double> &x, intptr_t dim)
{
	if (dim > x.ndim()) {
		throw error("Cannot access dimension % in Array with % dimensions", dim, x.ndim());
	}
	auto nrow = x.nrow();
	auto ncol = x.ncol();

	switch (dim)
	{
		case 1:
		{
			Array<double> result(nrow, 0.0);
			for (intptr_t i = 1; i <= nrow; i++)
			{
				double total = 0;
				for (intptr_t j = 1; j <= ncol; j++)
				{
					total += x(i,j);
				}
				result[i] = total;
			}
			return result;
		}
		case 2:
		{
			Array<double> result(ncol, 0.0);
			for (intptr_t j = 1; j <= ncol; j++)
			{
				double total = 0;
				for (intptr_t i = 1; i <= nrow; i++)
				{
					total += x(i,j);
				}
				result[j] = total;

			}
			return result;
		}
		default:
			throw error("sum not supported for arrays with % dimensions", dim);
	}
}

double mean(const Array<double> &x)
{
    return sum(x) / x.size();
}

Array<double> mean(const Array<double> &x, int dim)
{
	auto sums = sum(x, dim);
	auto size = dim == 1 ? x.ncol() : x.nrow();

	for (intptr_t i = 1; i <= sums.size(); i++) {
		sums[i] /= size;
	}

	return sums;
}

double maximum(const Array<double> &x)
{
	double result = x(1,1);

	for (intptr_t i = 1; i <= x.nrow(); i++)
	{
		for (intptr_t j = 1; j <= x.ncol(); j++)
		{
			auto value = x(i,j);
			if ((std::isfinite(value) && value > result) || !std::isfinite(result)) {
				result = value;
			}
		}
	}

	return result;
}

double minimum(const Array<double> &x)
{
	double result = x(1,1);

	for (intptr_t i = 1; i <= x.nrow(); i++)
	{
		for (intptr_t j = 1; j <= x.ncol(); j++)
		{
			auto value = x(i,j);
			if ((std::isfinite(value) && value < result) || !std::isfinite(result)) {
				result = value;
			}
		}
	}

	return result;
}


double sample_variance(const Array<double> &x)
{
    double mu = mean(x);
    intptr_t count = x.size();
    auto data = x.data();
    long double sum = 0;

    for (intptr_t i = 0; i < count; ++i)
    {
        sum += pow((data[i] - mu), 2);
    }

    return double(sum / (count - 1));
}

Array<double> sample_variance(const Array<double> &x, int dim)
{
	auto nrow = x.nrow();
	auto ncol = x.ncol();
	auto mu = mean(x, dim);

	switch (dim)
	{
		case 1:
		{
			Array<double> result(nrow, 0.0);

			for (intptr_t i = 1; i <= nrow; i++)
			{
				double total = 0;
				for (intptr_t j = 1; j <= ncol; j++)
				{
					total += pow(x(i,j) - mu[i], 2);
				}
				result[i] = total / (ncol - 1);
			}
			return result;
		}
		case 2:
		{
			Array<double> result(ncol, 0.0);

			for (intptr_t j = 1; j <= ncol; j++)
			{
				double total = 0;
				for (intptr_t i = 1; i <= nrow; i++)
				{
					total += pow(x(i,j) - mu[j], 2);
				}
				result[j] = total / (nrow - 1);

			}
			return result;
		}
		default:
			throw error("sum not supported for arrays with % dimensions", dim);
	}
}

double covariance(const Array<double> &x, const Array<double> &y)
{
	assert(x.ndim() == 1);
	assert(y.ndim() == 1);
	assert(x.size() == y.size());
	double cov = 0;
	double mu1 = mean(x);
	double mu2 = mean(y);

	for (intptr_t i = 1; i <= x.size(); i++)
	{
		cov += (x[i] - mu1) * (y[i] - mu2);
	}

	return cov / (x.size() - 1);
}

double pearson_correlation(const Array<double> &x, const Array<double> &y)
{
	return covariance(x, y) / (stdev(x) * stdev(y));
}

double stdev(const Array<double> &vector)
{
    return sqrt(sample_variance(vector));
}

Array<double> stdev(const Array<double> &x, int dim)
{
	auto var = sample_variance(x, dim);
	intptr_t count = var.size();
    auto data = var.data();

    for (intptr_t i = 0; i < count; ++i)
    {
        data[i] = sqrt(data[i]);
    }

    return var;
}

std::tuple<double,double,double> chi2_test(const Array<double> &data)
{
	if (data.ndim() != 2) {
		throw error("Chi squared test expected a two-dimensional array");
	}
	auto total = sum(data);
	auto row_sum = sum(data, 1);
	auto col_sum = sum(data, 2);
	double chi2 = 0;
	auto ncol = data.ncol();
	auto nrow = data.nrow();

	for (intptr_t j = 1; j <= ncol; j++)
	{
		for (intptr_t i = 1; i <= nrow; i++)
		{
			auto observed = data(i, j);
			auto expected = (row_sum[i] * col_sum[j]) / total;
			chi2 += pow(observed - expected, 2) / expected;
		}
	}
	auto df = (nrow - 1) * (ncol - 1);
	boost::math::chi_squared dist(df);
	auto p = 1 - boost::math::cdf(dist, chi2);

	return { chi2, df, p };
}

double t_statistic1(const Array<double> &x, double mu)
{
    return (mean(x) - mu) / (stdev(x) / sqrt(x.size()));
}

double t_statistic2(const Array<double> &x, const Array<double> &y, bool pooled)
{
    double mu1 = mean(x);
    double mu2 = mean(y);
    double var1 = sample_variance(x);
    double var2 = sample_variance(y);

    double std_err;
    if (pooled)
    {
    	auto se2 = ((x.size() - 1) * var1 + (y.size() - 1) * var2) / (x.size() + y.size() - 2);
    	std_err = sqrt(se2 / x.size() + se2 / y.size());
	    //std_err = sqrt(((x.size() - 1) * var1 + (y.size() - 1) * var2) / (x.size() + y.size() - 2));
    }
    else
		std_err = sqrt(var1 / x.size() + var2 / y.size());

    return (mu1 - mu2) / std_err;
}

static double compute_ttest(double t, double df, Alternative alt)
{
    boost::math::students_t dist(df);
    auto prob = cdf(dist, t);

    switch (alt)
    {
    	case Alternative::Greater:
    		return 1 - prob;
    	case Alternative::Less:
    		return prob;
    	default:
    		return 2 * prob;
    }
}

std::tuple<double, double, double> student_ttest1(const Array<double> &x, double mu, Alternative alt)
{
    auto t = t_statistic1(x, mu);
    double df = x.size() - 1;
    auto p = compute_ttest(t, df, alt);

    return { t, df, p };
}

std::tuple<double, double, double> student_ttest2(const Array<double> &x, const Array<double> &y, Alternative alt)
{
    auto t = t_statistic2(x, y, true);
    double df = x.size() + y.size() - 2;
    auto p = compute_ttest(t, df, alt);

    return { t, df, p };
}

std::tuple<double, double, double> welch_ttest2(const Array<double> &x, const Array<double> &y, Alternative alt)
{
    double mu1 = mean(x);
    double mu2 = mean(y);
    double var1 = sample_variance(x);
    double var2 = sample_variance(y);
    double size1 = x.size();
    double size2 = y.size();
    double nvar1 = var1 / size1;
    double nvar2 = var2 / size2;
    double std_err = sqrt(nvar1 + nvar2);

    auto t =  (mu1 - mu2) / std_err;
    double df = pow(nvar1 + nvar2, 2.0) / (pow(nvar1, 2.0) / (size1 - 1) + pow(nvar2, 2.0) / (size2 - 1));
    auto p = compute_ttest(t, df, alt);

    return { t, df, p };
}

double f_statistic(const Array<double> &x, const Array<double> &y)
{
	double var1 = sample_variance(x);
	double var2 = sample_variance(y);
	return  var1 / var2;
}

static double compute_f_test(double F, intptr_t df1, intptr_t df2)
{
	boost::math::fisher_f dist(df1, df2);
	return cdf(dist, F);
}

std::tuple<double,double,double,double> f_test(const Array<double> &x, const Array<double> &y, Alternative alt)
{
	intptr_t df1 = x.size() - 1;
	intptr_t df2 = y.size() - 1;
	double var1 = sample_variance(x);
	double var2 = sample_variance(y);
	double F = var1 / var2;
	double p;

	switch (alt)
	{
		case Alternative::Greater:
			p = compute_f_test(1./F, df2, df1);
			break;
		case Alternative::Less:
			p = compute_f_test(F, df1, df2);
			break;
		default:
			p = compute_f_test(F, df1, df2);
			p += 1 - compute_f_test(1./F, df2, df1);
			break;
	}

    return { F, df1, df2, p };
}

double kappa_fleiss(const Array<double> &ratings, intptr_t n)
{
    Array<double> proportions(1, ratings.ncol());
    Array<double> agreement(ratings.nrow(), intptr_t(1));
    double total = sum(ratings);

    // Proportion of scores assigned to each category
    for (int j = 1; j <= ratings.ncol(); j++)
    {
        double count = 0;

        for (intptr_t i = 1; i <= ratings.nrow(); i++)
        {
            count += ratings(i,j);
        }

        proportions(1,j) = count/total;
    }

    // Calculate agreement for the ith observation.
    for (int i = 1; i <= ratings.nrow(); i++)
    {
        double score = 0;

        for (intptr_t j = 1; j <= ratings.ncol(); j++)
        {
            score += pow(ratings(i,j), 2);
        }

        agreement(i, 1) = (score - n) / (n * (n-1));
    }

    // Sum of squared proportions.
    double prop = 0;

    for (intptr_t j = 1; j <= proportions.ncol(); j++)
    {
        prop += pow(proportions(1,j), 2);
    }

    double kappa = (mean(agreement) - prop) / (1 - prop);

    return kappa;
}

}} // namespace phonometrica::stats
