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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <boost/math/distributions/students_t.hpp>
#include <boost/math/distributions/chi_squared.hpp>
#include <phon/analysis/regression.hpp>
#include <phon/analysis/statistics.hpp>
#include <phon/utils/matrix.hpp>
#include <phon/third_party/LBFGSpp/LBFGS.h>

namespace phonometrica::stats {

LinearModel lm(const Array<double> &y, const Array<double> &X)
{
	using namespace Eigen;

	if (y.ndim() != 1) {
		throw error("y must a one-dimensional array");
	}
	if (X.ndim() != 2) {
		throw error("X must be a two-dimensional array");
	}
	if (X.nrow() != y.size()) {
		throw error("Inconsistent number of observations in y and X");
	}
	intptr_t m = X.ncol();
	intptr_t n = X.nrow();

	if (n <= m) {
		throw error("Not enough data points to perform linear regression");
	}

	Array<double> beta(m, 0.0), se(m, 0.0), t(m, 0.0), p(m, 0.0);
	Map<Matrix<double>> X1(const_cast<double*>(X.data()), X.nrow(), X.ncol());
	Map<Vector<double>> y1(const_cast<double*>(y.data()), y.size());
	Map<Vector<double>> b1(beta.data(), beta.size());

	BDCSVD<Matrix<double>> svd(X1, ComputeThinU|ComputeThinV);
	b1 = svd.solve(y1);

	Array<double> yhat(n, 0.0);  // predicted values
	Array<double> resid(n, 0.0); // residuals

	for (intptr_t i = 1; i <= n; i++)
	{
		double val = 0.0;

		for (intptr_t j = 1; j <= m; j++)
		{
			val += X(i,j) * beta[j];
		}
		yhat[i] = val;
	}

	// Estimate residual variance
	intptr_t df = n - m;
	long double sse = 0.0; // sum of squared errors

	for (intptr_t i = 1; i <= n; i++)
	{
		auto e = y[i] - yhat[i];
		resid[i] = e;
		sse += e * e;
	}
	auto rv = sse / df;

	// Get standard errors, t-values and p-values
	auto var = (X1.transpose() * X1).inverse();

	boost::math::students_t dist(df);

	for (intptr_t i = 1; i <= m; i++)
	{
		// Standard error
		se[i] = sqrt(rv * var(i-1, i-1));
		// t-value
		t[i] = beta[i] / se[i];
		// p-value
		p[i] = 2 * (1 - cdf(dist, std::abs(t[i])));
	}

	// R^2
	double ybar = mean(y);
	long double ssr = 0.0; // sum of squared residuals
	long double sst = 0.0; // total sum of squares

	for (intptr_t i = 1; i <= n; i++)
	{
		ssr += resid[i] * resid[i];
		sst += (y[i] - ybar) * (y[i] - ybar);
	}
	auto r2 = 1 - double(ssr / sst);
	int np = m - 1; // number of predictors
	double adj_r2 = 1 - (1 - r2) * (double(n - 1) / (n - np - 1));

	return { beta, se, t, p, yhat, resid, double(sqrt(rv)), df , r2, adj_r2 };
}

static Vector<double> sigmoid(const Array<double> &predictors, const Vector<double> &b)
{
	Eigen::Map<Matrix<double>> X(const_cast<double*>(predictors.data()), predictors.nrow(), predictors.ncol());
	return  1 / (1 + exp(-(X*b).array()));
}

static void gradient_update(const Array<double> &predictors, const Array<double> &response, const Vector<double> &h, Vector<double> &grad)
{
	intptr_t n = response.size();
	Eigen::Map<Vector<double>> y(const_cast<double*>(response.data()), n);
	Eigen::Map<Matrix<double>> X(const_cast<double*>(predictors.data()), predictors.nrow(), predictors.ncol());
	grad = (X.transpose() * (h - y)).array() / n;
}

static double logit_cost(const Array<double> &response, const Vector<double> &h)
{
	intptr_t n = response.size();
	Eigen::Map<Vector<double>> y(const_cast<double*>(response.data()), n);
	auto ya = y.transpose().array();
	auto ha = h.array();

	Matrix<double> result = ((-ya).matrix() * log(ha).matrix() - (1 - ya).matrix() * log (1 - ha).matrix());
	assert(result.rows() == 1);
	assert(result.cols() == 1);

	return result(0, 0) / n;
}

static GLModel glm(const Array<double> &y, const Array<double> &X, int max_iter,
		const std::function<double(const Vector<double> &beta, Vector<double> &grad)> &cost_func,
		const std::function<Matrix<double>(const Array<double> &X, const Vector<double> &beta)> &cov_func)
{
	using namespace LBFGSpp;

	if (y.ndim() != 1)
	{
		throw error("y must a one-dimensional array");
	}
	if (X.ndim() != 2)
	{
		throw error("X must be a two-dimensional array");
	}
	if (X.nrow() != y.size())
	{
		throw error("Inconsistent number of observations in y and X");
	}
	intptr_t m = X.ncol();
	intptr_t n = X.nrow();

	if (n <= m)
	{
		throw error("Not enough data points to perform regression");
	}

	Eigen::VectorXd weights = Eigen::VectorXd::Zero(m);
	LBFGSParam<double> param;
	param.epsilon = 1e-6;
	param.max_iterations = max_iter;
	LBFGSSolver<double> solver(param);
	double fx;
	int niter = solver.minimize(cost_func, weights, fx);
	bool converged = (niter < param.max_iterations);

	Array<double> beta(m, 0.0);
	std::copy(weights.data(), weights.data() + m, beta.data());

	// Variance-covariance matrix
	Matrix<double> cov = cov_func(X, weights);
	assert(cov.rows() == m);
	Array<double> se(m, 0.0);

	for (intptr_t i = 0; i < m; i++) {
		se[i+1] = sqrt(cov(i,i));
	}

	// z-values
	Array<double> z(m, 0.0);

	for (intptr_t i = 1; i <= m; i++) {
		z[i] = beta[i] / se[i];
	}

	// p-values for a Wald test
	boost::math::chi_squared dist(1); // chi-squared distribution with one degree of freedom
	Array<double> p(m, 0.0);

	for (intptr_t i = 1; i <= m; i++)
	{
		auto stat = (beta[i] * beta[i]) / cov(i-1, i-1);
		p[i] = 1 - boost::math::cdf(dist, stat);
	}

	return { beta, se, z, p, niter, converged };
}

GLModel logit(const Array<double> &y, const Array<double> &X, int max_iter)
{
	for (auto value : y)
	{
		if (value != 0 && value != 1) {
			throw error("Response array can only contain the values 0 and 1");
		}
	}

	auto cost = [&](const Eigen::VectorXd &b, Eigen::VectorXd &grad)
	{
		auto h = sigmoid(X, b);
		gradient_update(X, y, h, grad);

		return logit_cost(y, h);
	};

	auto covar = [&](const Array<double> &X, const Vector<double> &beta) -> Matrix<double>
	{
		intptr_t n = X.nrow();
		Vector<double> fitted = sigmoid(X, beta);
		Vector<double> W = Eigen::VectorXd::Zero(n);
		Eigen::Map<Matrix<double>> X2(const_cast<double*>(X.data()), X.nrow(), X.ncol());
		for (intptr_t i = 0; i < n; i++) {
			W[i] = fitted[i] * (1 - fitted[i]);
		}

		return (X2.transpose() * W.asDiagonal() * X2).inverse();

	};

	return glm(y, X, max_iter, cost, covar);
}

static Vector<double> poisson_mean(const Array<double> &predictors, const Vector<double> &b)
{
	Eigen::Map<Matrix<double>> X(const_cast<double*>(predictors.data()), predictors.nrow(), predictors.ncol());
	return  exp((X*b).array());
}

static double poisson_cost(const Array<double> &response, const Vector<double> &h)
{
	intptr_t n = response.size();
	Eigen::Map<Vector<double>> y(const_cast<double*>(response.data()), n);
	// FIXME: check this code, the transpose is not used
	auto ya = y.transpose().array();
	auto ha = h.array();

	return (h.array() - (y.array() * log(h.array()))).sum();
}

GLModel poisson(const Array<double> &y, const Array<double> &X, bool robust, int max_iter)
{
	auto cost = [&](const Eigen::VectorXd &b, Eigen::VectorXd &grad)
	{
		auto h = poisson_mean(X, b);
		gradient_update(X, y, h, grad);

		return poisson_cost(y, h);
	};

	if (robust)
	{
		auto cov = [&](const Array<double> &X, const Vector<double> &beta) -> Matrix<double> {
			intptr_t n = X.nrow();
			Eigen::Map<Matrix<double>> X2(const_cast<double*>(X.data()), X.nrow(), X.ncol());
			Vector<double> W = poisson_mean(X, beta);
			Vector<double> W2 = Eigen::VectorXd::Zero(n);
			for (intptr_t i = 0; i < n; i++)
			{
				double e = y[i+1] - W[i];
				W2[i] = e * e;
			}
			auto XT = X2.transpose();
			auto Wdiag = W.asDiagonal();
			auto exp = (XT * Wdiag * X2).inverse();

			return exp * (XT * W2.asDiagonal() * X2) * exp;
		};

		return glm(y, X, max_iter, cost, cov);
	}
	else
	{
		auto cov = [&](const Array<double> &X, const Vector<double> &beta) -> Matrix<double> {
			Eigen::Map<Matrix<double>> X2(const_cast<double*>(X.data()), X.nrow(), X.ncol());
			Vector<double> W = poisson_mean(X, beta);

			return (X2.transpose() * W.asDiagonal() * X2).inverse();
		};

		return glm(y, X, max_iter, cost, cov);
	}
}

} // namespace phonometrica::stats
