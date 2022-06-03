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
 * Created: 05/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: matrix type built on top of Eigen.                                                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_MATRIX_HPP
#define PHONOMETRICA_MATRIX_HPP

#include <functional>
#include <phon/array.hpp>
#include <phon/third_party/Eigen/Dense>

namespace phonometrica {

template <typename T>
using Matrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>;

template <typename T>
using Vector = Eigen::Matrix<T, Eigen::Dynamic, 1, Eigen::ColMajor>;

// Convert an input matrix element-wise using formula.
Array<double> apply(const Array<double> &X, const std::function<double(double)> &formula);

Array<double> transpose(const Array<double> &X);

Array<double> mul(const Array<double> &X, const Array<double> &Y);

Array<double> mul(const Array<double> &X, double n);

Array<double> div(const Array<double> &X, double n);

Array<double> add(const Array<double> &X, double n);

Array<double> sub(const Array<double> &X, double n);

Array<double> operator-(const Array<double> &x, const Array<double> &y);

} // namespace phonometrica

#endif // PHONOMETRICA_MATRIX_HPP
