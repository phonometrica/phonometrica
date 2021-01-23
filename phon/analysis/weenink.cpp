/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * Created: 06/11/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/file.hpp>
#include <phon/analysis/weenink.hpp>
#include <phon/analysis/speech_utils.hpp>
#include <phon/application/sound.hpp>

#define MIN_POINTS 8

namespace phonometrica::speech {

double legendre(double x, unsigned int n)
{
	assert(x >= -1.0 && x <= 1.0);

	switch (n)
	{
		case 0:
			return 1.0;
		case 1:
			return x;
		case 2:
			return 0.5 * (3 * x * x - 1);
		case 3:
			return 0.5 * (5 * pow(x, 3) - 3 * x);
		case 4:
			return 0.25 * (35 * pow(x, 4) - 30 * x * x + 3);
		case 5:
			return 0.25 * (63 * pow(x, 5) - 70 * pow(x, 3) + 15 * x);
		case 6:
			return 0.0625 * (231 * pow(x, 6) - 315 * pow(x, 4) + 105 * x * x - 5);
		case 7:
			return 0.0625 * (429 * pow(x, 7) - 693 * pow(x, 5) + 315 * pow(x, 3) - 35 * x);
		default:
			throw error("Legendre polynomial degree too high");
	}
}

WeeninkModel model_segment(const Matrix<double> &F, const Matrix<double> &B, unsigned int p)
{
	// Get number of data points and number of formants
	auto ndata = F.rows();
	auto nformant = F.cols();
	assert(ndata >= MIN_POINTS);
	WeeninkModel model(nformant, p);

	// Normalize time on the interval [-1 1]
	auto time = linspace(-1, 1, ndata);

	// Compute the smoothness of each formant
	for (int k = 0; k < nformant; ++k)
	{
		// Reset in case there were undefined values in the previous column.
		ndata = F.rows();

		// First check for undefined formants. Undefined formants are not taken into account in the computation of smoothness,
		// But we need to make sure they are evaluated at the correct time point.
		std::vector<unsigned int> time_index; // map row index in the design matrix to index in formant/bandwidth matrices
		time_index.reserve(F.rows());

		for (int i = 0; i < ndata; i++)
		{
			if (!std::isnan(F(i,k))) {
				time_index.push_back(i);
			}
		}
		ndata = time_index.size();

		if (ndata <= 2 * nformant) {
			return model; // Failed: too few observations.
		}

		// Create the design matrix as an nxp design matrix, where n is the number of data points and p the number of polynomials.
		Matrix<double> A(ndata, p);
		A.setZero();

		for (int i = 0; i < ndata; ++i)
		{
			for (int j = 0; j < p; ++j)
			{
				auto index = time_index[i];
				// Evaluate Legendre polynomial L(j) at time i
				auto l = legendre(time[index], j);
				A(i,j) = l / B(index,k);
			}
		}

		// Normalize Observed frequencies by their bandwidth
		Vector<double> b(ndata);

		for (int i = 0; i < ndata; ++i)
		{
			auto index = time_index[i];
			b[i] = F(index,k) / B(index,k);
		}

		// Obtain the singular value decomposition of A
		Eigen::BDCSVD<Matrix<double>> svd(A, Eigen::ComputeThinU|Eigen::ComputeThinV);

		// Estimate the parameters of the model
		Vector<double> beta = svd.solve(b);

		for (unsigned int i = 0; i < p; i++) {
			model.coeff(i,k) = beta(i);
		}

		// Estimate variance of the parameters
		auto S = svd.singularValues();
		auto V = svd.matrixV();

		for (int j = 0; j < p; ++j)
		{
			double value = 0.0;

			for (int i = 0; i < p; ++i)
			{
				auto tmp = V(j,i) / S(i);
				value +=  tmp * tmp;
			}

			model.var(j, k) = value;
		}

		// Estimate goodness-of-fit
		double chi2 = 0;

		for (int i = 0; i < ndata; i++)
		{
			auto index = time_index[i];
			double f_hat = model.predict(time[index], k);
			auto x = (F(index, k) - f_hat) / B(index,k);
			chi2 += x*x;
		}

		model.observations[k] = ndata;
		model.chi2(k) = chi2;
	}
	model.success = true;

	return model;
}

double WeeninkModel::predict(double t, unsigned int formant) const
{
	double result = 0;

	for (int i = 0; i < p; i++)
	{
		result += coeff(i, formant) * legendre(t, i);
	}

	return result;
}

double WeeninkModel::score(double t) const
{
	// Average variance;
	double s2 = var.sum() / var.size();
	// Summed chi2 error over all formants
	double x2 = chi2.sum();
	// Degrees of freedom of chi2 over all formants
	unsigned int d = 0;

	for (auto n : observations) {
		d += n - 1;
	}

	return pow(s2, t) * (x2 / d);
}

std::pair<double,double> find_lpc_parameters(Sound *sound, int nformant, double max_bandwidth, double win_size,
		double t1, double t2, double max_freq1, double max_freq2, double step, int lpc_order1, int lpc_order2)
{
	// Take at least 8 points, or about one measurement every 5 ms.
	int npoint = 1000 * (t2 - t1) / 5;
	if (npoint < MIN_POINTS) npoint = MIN_POINTS;
	auto time_points = linspace(t1, t2, npoint, false);
	double best_score = (std::numeric_limits<double>::max)();
	std::pair<double,double> best_parameters;

	Matrix<double> F(npoint, nformant); // formants (each column is a formant track)
	Matrix<double> B(npoint, nformant); // bandwidths
	double nyquist = max_freq1;

	while (nyquist <= max_freq2)
	{
		for (int order = lpc_order1; order <= lpc_order2; order++)
		{
			// Calculate formant tracks for the given parameters.
			F.setZero(npoint, nformant);
			B.setZero(npoint, nformant);
			intptr_t i = 0;

			for (auto t : time_points)
			{
				auto formants = sound->get_formants(t, nformant, nyquist, max_bandwidth, win_size, order);

				for (intptr_t j = 0; j < nformant; j++)
				{
					F(i,j) = formants(j+1, 1);
					B(i,j) = formants(j+1, 2);
				}

				i++;
			}

			// Model formants
			auto model = model_segment(F, B);
			if (!model.success) continue;
			auto W = model.score();

			if (std::isfinite(W) && W < best_score)
			{
				best_score = W;
				best_parameters = { nyquist, order };
			}
		}

		nyquist += step;
	}

	return best_parameters;
}

} // namespace phonometrica::speech