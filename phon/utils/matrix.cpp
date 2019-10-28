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
 * Created: 26/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/utils/matrix.hpp>

namespace phonometrica {

Array<double> apply(const Array<double> &X, const std::function<double(double)> &formula)
{
	Array<double> Y(X.nrow(), X.ncol());

	for (intptr_t i = 1; i <= X.size(); i++) {
		Y[i] = formula(X[i]);
	}

	return Y;
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

} // namespace phonometrica