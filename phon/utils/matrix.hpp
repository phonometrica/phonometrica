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
Array<double> apply(const Array<double> &input, const std::function<double(double)> &formula);

Array<double> transpose(const Array<double> &X);

Array<double> mul(const Array<double> &X, const Array<double> &Y);

Array<double> mul(const Array<double> &X, double n);

Array<double> div(const Array<double> &X, double n);

Array<double> add(const Array<double> &X, double n);

Array<double> sub(const Array<double> &X, double n);

} // namespace phonometrica

#endif // PHONOMETRICA_MATRIX_HPP
