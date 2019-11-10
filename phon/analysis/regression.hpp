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
 * Created: 08/11/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: linear regression.                                                                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_REGRESSION_HPP
#define PHONOMETRICA_REGRESSION_HPP

#include <functional>
#include <phon/string.hpp>
#include <phon/array.hpp>

namespace phonometrica::stats {

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

// Generalized linear model
struct GLModel
{
	Array<double> beta;      // regression coefficients
	Array<double> se;        // standard errors
	Array<double> z;         // z-values
	Array<double> p;         // p-values for Wald test
	int niter;               // number of iterations
	bool converged;          // whether the model converged
};


//! Performs linear regression using the least-squared method.
//! \param y a vector of N observations
//! \param X an N by M matrix, where N is the number of observations and M the number of regression coefficients. The
// first column contains the intercept (beta_0).
//! \return a vector of N coefficient (the first coefficient is the intercept).

LinearModel lm(const Array<double> &y, const Array<double> &X);


// Logistic regression
GLModel logit(const Array<double> &y, const Array<double> &X, int max_iter = 200);


// Poisson regression
GLModel poisson(const Array<double> &y, const Array<double> &X, int max_iter);

} // namespace phonometrica::stats

#endif // PHONOMETRICA_REGRESSION_HPP
