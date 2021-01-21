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
 * Created: 26/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/utils/matrix.hpp>

namespace phonometrica {

Array<double> apply(const Array<double> &X, const std::function<double(double)> &formula)
{
	if (X.ndim() == 1)
	{
		Array<double> Y(X.size(), 0.0);

		for (intptr_t i = 1; i <= X.size(); i++) {
			Y[i] = formula(X[i]);
		}

		return Y;
	}
	else
	{
		Array<double> Y(X.nrow(), X.ncol());

		for (intptr_t i = 1; i <= X.size(); i++) {
			Y[i] = formula(X[i]);
		}

		return Y;
	}
}

Array<double> transpose(const Array<double> &X)
{
	if (X.ndim() != 2) {
		throw error("Cannot transpose array with % dimensions", X.ndim());
	}
	Array<double> Y(X.ncol(), X.nrow());
	Eigen::Map<Matrix<double>> X1(const_cast<double*>(X.data()), X.nrow(), X.ncol());
	Eigen::Map<Matrix<double>> Y1(const_cast<double*>(Y.data()), Y.nrow(), Y.ncol());
	Y1 = X1.transpose();

	return Y;
}

Array<double> mul(const Array<double> &X, const Array<double> &Y)
{
	Array<double> Z(X.nrow(), Y.ncol());
	Eigen::Map<Matrix<double>> X1(const_cast<double*>(X.data()), X.nrow(), X.ncol());
	Eigen::Map<Matrix<double>> Y1(const_cast<double*>(Y.data()), Y.nrow(), Y.ncol());
	Eigen::Map<Matrix<double>> Z1(const_cast<double*>(Z.data()), Z.nrow(), Z.ncol());
	Z1 = X1 * Y1;

	return Z;
}

Array<double> mul(const Array<double> &X, double n)
{
	Array<double> Y(X.nrow(), X.ncol());

	for (intptr_t i = 1; i <= X.size(); i++) {
		Y[i] = X[i] * n;
	}

	return Y;
}

Array<double> div(const Array<double> &X, double n)
{
	Array<double> Y(X.nrow(), X.ncol());

	for (intptr_t i = 1; i <= X.size(); i++) {
		Y[i] = X[i] / n;
	}

	return Y;
}

Array<double> add(const Array<double> &X, double n)
{
	Array<double> Y(X.nrow(), X.ncol());

	for (intptr_t i = 1; i <= X.size(); i++) {
		Y[i] = X[i] + n;
	}

	return Y;
}

Array<double> sub(const Array<double> &X, double n)
{
	Array<double> Y(X.nrow(), X.ncol());

	for (intptr_t i = 1; i <= X.size(); i++) {
		Y[i] = X[i] - n;
	}

	return Y;
}

Array<double> operator-(const Array<double> &x, const Array<double> &y)
{
	x.check_dim(y);
	Array<double> z(x);
	auto it1 = x.begin();
	auto it2 = y.begin();
	auto it3 = z.begin();

	while (it1 != x.end()) {
		*it3++ = *it1++ - *it2++;
	}

	return z;
}

} // namespace phonometrica