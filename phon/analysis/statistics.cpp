/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
 *                                                                                                                     *
 * Created: 28/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <math.h>
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

double sample_variance(const Array<double> &x)
{
    double mu = mean(x);
    intptr_t count = x.size();
    auto data = x.data();
    double sum = 0;

    for (intptr_t i = 0; i < count; ++i)
    {
        sum += pow((data[i] - mu), 2.0);
    }

    return sum / (count - 1);
}

double stdev(const Array<double> &vector)
{
    return sqrt(sample_variance(vector));
}

double t_statistic1(const Array<double> &vector, double mu)
{
    return (mean(vector) - mu) / (stdev(vector) / sqrt(vector.size()));
}

double t_statistic2(const Array<double> &vector1, const Array<double> &vector2)
{
    double mu1 = mean(vector1);
    double mu2 = mean(vector2);
    double var1 = sample_variance(vector1);
    double var2 = sample_variance(vector2);
    double std_err = sqrt(var1/vector1.size() + var2/vector2.size());
    // FIXME: pooled variance for Student's t-test?

    return (mu1 - mu2) / std_err;
}

double f_statistic(const Array<double> &vector1, const Array<double> &vector2)
{
    return sample_variance(vector1) / sample_variance(vector2);
}

// For now, equal variance, two-sided
static inline double compute_ttest(double t, double df, bool double_sided)
{
    boost::math::students_t dist(df);
    int coeff = double_sided ? 2 : 1;

    return cdf(complement(dist, std::abs(t))) * coeff;
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

double student_ttest1(const Array<double> &vector, double mu, double &t, bool double_sided)
{
    t = t_statistic1(vector, mu);
    double df = vector.size() - 1;

    return compute_ttest(t, df, double_sided);
}

double student_ttest2(const Array<double> &vector1, const Array<double> &vector2, double &t, bool double_sided)
{
    t = t_statistic2(vector1, vector2);
    double df = vector1.size() + vector2.size() - 2;

    return compute_ttest(t, df, double_sided);
}

double welch_ttest2(const Array<double> &vector1, const Array<double> &vector2, double &t, bool double_sided)
{
    double mu1 = mean(vector1);
    double mu2 = mean(vector2);
    double var1 = sample_variance(vector1);
    double var2 = sample_variance(vector2);
    double size1 = vector1.size();
    double size2 = vector2.size();
    double nvar1 = var1 / size1;
    double nvar2 = var2 / size2;
    double std_err = sqrt(nvar1 + nvar2);

    t =  (mu1 - mu2) / std_err;
    double df = pow(nvar1 + nvar2, 2.0) / (pow(nvar1, 2.0) / (size1 - 1) + pow(nvar2, 2.0) / (size2 - 1));

    return compute_ttest(t, df, double_sided);
}


double f_test(const Array<double> &vector1, const Array<double> &vector2, double &F)
{
    intptr_t df1 = vector1.size() - 1;
    intptr_t df2 = vector2.size() - 1;
    F = f_statistic(vector1, vector2);
    boost::math::fisher_f dist(df1, df2);

    return cdf(dist, F) * 2; // double sided
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
