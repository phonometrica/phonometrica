/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any      *
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
 * Purpose: routines for statistical analysis.                                                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_STATISTICS_HPP
#define PHONOMETRICA_STATISTICS_HPP

#include <phon/array.hpp>

namespace phonometrica { namespace stats {

enum class Alternative
{
	TwoTailed,
	Greater,
	Less
};

struct LinearModel
{
	Array<double> beta;      // regression coefficients
	Array<double> se;        // standard errors
	Array<double> t;         // t-values
	Array<double> p;         // p-values
	Array<double> predicted; // predicted values
	Array<double> residuals; // residual errors
	double rse;              // residual standard error
	intptr_t df;             // degrees of freedom
	double r2;               // R squared
	double adj_r2;           // Adjusted R squared
};

double sum(const Array<double> &x);

Array<double> sum(const Array<double> &x, intptr_t dim);

double mean(const Array<double> &x);

Array<double> mean(const Array<double> &x, int dim);

double sample_variance(const Array<double> &x);

double covariance(const Array<double> &x, const Array<double> &y);

double pearson_correlation(const Array<double> &x, const Array<double> &y);

Array<double> sample_variance(const Array<double> &x, int dim);

double stdev(const Array<double> &vector);

Array<double> stdev(const Array<double> &x, int dim);

// One-sample t-test
double t_statistic1(const Array<double> &x, double mu);

// Two-sample t-test
double t_statistic2(const Array<double> &x, const Array<double> &y);

double f_statistic(const Array<double> &x, const Array<double> &y);

// Pearson's chi-squared test
std::tuple<double,double,double> chi2_test(const Array<double> &data);

// Student's one-sample t-test
std::tuple<double, double, double> student_ttest1(const Array<double> &x, double mu, Alternative alt = Alternative::TwoTailed);

// Student's two-sample t-test (equal variance)
std::tuple<double, double, double>
student_ttest2(const Array<double> &x, const Array<double> &y, Alternative alt = Alternative::TwoTailed);

// Welch's two-sample t-test (unequal variance)
std::tuple<double, double, double>
welch_ttest2(const Array<double> &x, const Array<double> &y, Alternative alt = Alternative::TwoTailed);


std::tuple<double,double,double,double>
f_test(const Array<double> &x, const Array<double> &y, Alternative alt = Alternative::TwoTailed);

// Fleiss' Kappa for inter-rater agreement.
// `ratings` is an n by k matrix, with n observations and k categories. Each (n,k) cell represents the number of raters
// that assigned observation n to category k. `n` is the number of raters.
double kappa_fleiss(const Array<double> &ratings, intptr_t n);


//! Performs linear regression using the least-squared method.
//! \param y a vector of N observations
//! \param X an N by M matrix, where N is the number of observations and M the number of regression coefficients. The
// first column contains the intercept (beta_0).
//! \return a vector of N coefficient (the first coefficient is the intercept).

LinearModel lm(const Array<double> &y, const Array<double> &X);

}} // namespace phonometrica::stats

#endif // PHONOMETRICA_STATISTICS_HPP
