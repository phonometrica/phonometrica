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
 * Created: 06/11/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Implement Weenink's method for automatic formant measurement. See:                                         *
 * Weenink, D. J. M. (2015). "Improved formant frequency measurements of short segments". In Proceedings of the 18th   *
 * International Congress of Phonetic Sciences Glasgow: The University of Glasgow.                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_WEENINK_HPP
#define PHONOMETRICA_WEENINK_HPP

#include <cassert>
#include <iostream>
#include "third_party/Eigen/Dense"
#include "third_party/Eigen/SVD"
#include <phon/string.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/utils/matrix.hpp>

namespace phonometrica {
class Sound;
}

namespace phonometrica::speech {

class WeeninkModel final
{
public:

	WeeninkModel(unsigned int formants, int p) :
		coeff(p, formants), var(p, formants), observations(formants, 0), chi2(formants), p(p)
	{ }

	// Predict formant value at time t based on a fitted model
	double predict(double t, unsigned int formant) const;

	// Score the fitted model
	double score(double t = 1.2) const;

	unsigned int formant_count() const { return (unsigned int) coeff.cols(); }

	// An MxN matrix: rows represent parameter values (one per Legendre polynomial) and columns represent measured formants.
	Matrix<double> coeff;

	// An MxN matrix where each cell var(i,j) contains the variance of parameter param(i,j).
	Matrix<double> var;

	// Number of observations (excluding undefined values) for each formant.
	std::vector<unsigned int> observations;

	// A vector of goodness-of-fit values (1 per formant)
	Vector<double> chi2;

	// Number of polynomials
	int p;

	// Whether smoothness could be properly estimated.
	bool success = false;
};


/**
 * Evaluate Legendre polynomial of degree n (up to degree 7).
 * @param x value to be evaluated
 * @param n degree of the polynomial
 * @return y value
 */
double legendre(double x, unsigned int n);

/**
 * Model a vocoid segment's formants using Weenink's method. Each formant track is modeled using a linear combination
 * of Legendre polynomials.
 * @param F an NxM matrix with M formants (typically F1, F2, F3) measured at N time points
 * @param B an NxM matrix where each cell (i,j) contains the bandwidth corresponding to formant F(i,j)
 * @param p the number of Legendre polynomial to be included in the model, starting at degree 0
 * @return a model object for the segment
 */
WeeninkModel model_segment(const Matrix<double> &F, const Matrix<double> &B, unsigned int p = 4);

// Find the best <Nyquist frequency, LPC order> pair for a vocoid given a set of parameter to search for.
std::pair<double,double> find_lpc_parameters(Sound *sound, int nformant, double max_bandwidth, double win_size,
		double t1, double t2, double max_freq1, double max_freq2, double step, int lpc_order1, int lpc_order2);


} // namespace phonometrica::speech

#endif // PHONOMETRICA_WEENINK_HPP
