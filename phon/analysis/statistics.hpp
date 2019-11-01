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


}} // namespace phonometrica::stats

#endif // PHONOMETRICA_STATISTICS_HPP
