// ISC License
//
// Copyright (c) 2013--2017, librosa development team.
//               2019, Julien Eychenne
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.
//
// Note: The Burg method is a C++ port of the Python implementation in librosa 0.7.1. See:
// https://github.com/librosa/librosa/blob/master/librosa/core/audio.py

// SPTK's implementation seems to get better results.

/* Sound_to_Formant.cpp
 *
 * Copyright (C) 1992-2011,2014,2015,2016,2019 Paul Boersma
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this work. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * pb 2002/07/16 GPL
 * pb 2003/05/15 replaced memcof with NUMburg
 * pb 2003/09/18 default time step is 4 times oversampling
 * pb 2006/05/10 better handling of interruption in Sound_to_Formant
 * pb 2006/05/10 better handling of NULL from Polynomial_to_Roots
 * pb 2007/01/26 made compatible with stereo Sounds
 * pb 2007/03/30 changed float to double (against compiler warnings)
 * pb 2010/12/13 removed some style bugs
 * pb 2011/06/08 C++
 */


#define USE_SPTK 1

#if USE_SPTK
#include <SPTK.h>
#else
#include <numeric>
#include <vector>
#endif

#include <phon/utils/matrix.hpp>

namespace phonometrica::speech {

/* Childers (1978), Modern Spectrum analysis, IEEE Press, 252-255) */
/* work [1..n+n+n];
b1 = & work [1];
b2 = & work [n+1];
aa = & work [n+n+1];
for (i=1; i<=n+n+n; i ++) work [i]=0;
*/


Vector<double> get_lpc_coefficients(const Array<double> &x, int order)
#if 0
{
	// TODO: take into account errors in LPC.
	Vector<double> coefficients(order);
	coefficients.setZero(order);
	intptr_t n = x.size(), m = coefficients.size();

	Array<double> b1(n, 0.0), b2(n, 0.0), aa(m, 0.0);

	// (3)

	long double p = 0.0;
	for (intptr_t j = 1; j <= n; j++)
		p += x[j] * x[j];

	long double xms = p / n;
	if (xms <= 0.0)
		return coefficients;    // warning empty

	// (9)

	b1[1] = x[1];
	b2[n - 1] = x[n];
	for (intptr_t j = 2; j <= n - 1; j++)
		b1[j] = b2[j - 1] = x[j];

	for (intptr_t i = 1; i <= m; i++)
	{
		// (7)

		long double num = 0.0, denum = 0.0;
		for (intptr_t j = 1; j <= n - i; j++)
		{
			num += b1[j] * b2[j];
			denum += b1[j] * b1[j] + b2[j] * b2[j];
		}

		if (denum <= 0.0)
			return coefficients;    // warning ill-conditioned

		coefficients[i-1] = 2.0 * num / denum;

		// (10)

		xms *= 1.0 - coefficients[i-1] * coefficients[i-1];

		// (5)

		for (intptr_t j = 1; j <= i - 1; j++)
			coefficients[j-1] = aa[j] - coefficients[i-1] * aa[i - j];

		if (i < m)
		{

			// (8) Watch out: i -> i+1

			for (intptr_t j = 1; j <= i; j++)
				aa[j] = coefficients[j-1];
			for (intptr_t j = 1; j <= n - i - 1; j++)
			{
				b1[j] -= aa[i] * b2[j];
				b2[j] = b2[j + 1] - aa[i] * b1[j + 1];
			}
		}
	}

	return coefficients;
}

#else // 0
{
	assert(order > 1);

#if USE_SPTK
	Vector<double> coeff(order + 1);
	lpc(const_cast<double*>(x.data()), x.size(), coeff.data(), order, 0.000001);

	return coeff;
#else

	// This implementation follows the description of Burg's algorithm given in
	// section III of Marple's paper:
	//     Larry Marple, A New Autoregressive Spectrum Analysis Algorithm
	//     IEEE Transactions on Accoustics, Speech, and Signal Processing
	//     vol 28, no. 4, 1980.
	//
	// We use the Levinson-Durbin recursion to compute AR coefficients for each
	// increasing model order by using those from the last. We maintain two
	// arrays and then flip them each time we increase the model order so that
	// we may use all the coefficients from the previous order while we compute
	// those for the new one. These two arrays hold ar_coeffs for order M and
	// order M-1.  (Corresponding to a_{M,k} and a_{M-1,k} in eqn 5)
	std::vector<double> ar_coeffs(order + 1);
	std::vector<double> ar_coeffs_prev(order + 1);
	ar_coeffs[0] = 1;
	ar_coeffs_prev[0] = 1;

	// These two arrays hold the forward and backward prediction error. They
	// correspond to f_{M-1,k} and b_{M-1,k} in eqns 10, 11, 13 and 14 of
	// Marple. First they are used to compute the reflection coefficient at
	// order M from M-1 then are re-used as f_{M,k} and b_{M,k} for each
	// iteration of the below loop
	std::vector<double> fwd_pred_error(x.begin() + 1, x.end());
	std::vector<double> bwd_pred_error(x.begin(), x.end() - 1);

	// DEN_{M} from eqn 16 of Marple.
	auto dot1 = std::inner_product(fwd_pred_error.begin(), fwd_pred_error.end(), fwd_pred_error.begin(), 0.0);
	auto dot2 = std::inner_product(bwd_pred_error.begin(), bwd_pred_error.end(), bwd_pred_error.begin(), 0.0);
	auto den = dot1 + dot2;

	for (int i = 0; i < order; i++)
	{
		if (den <= 0) {
			throw error("[Numerical error] Input ill-conditioned?");
		}

		// Eqn 15 of Marple, with fwd_pred_error and bwd_pred_error
		// corresponding to f_{M-1,k+1} and b{M-1,k} and the result as a_{M,M}
		//reflect_coeff = dtype(-2) * np.dot(bwd_pred_error, fwd_pred_error) / dtype(den)
		auto reflect_coeff = -2 * std::inner_product(bwd_pred_error.begin(), bwd_pred_error.end(), fwd_pred_error.begin(), 0.0) / den;

		// Now we use the reflection coefficient and the AR coefficients from
		// the last model order to compute all of the AR coefficients for the
		// current one.  This is the Levinson-Durbin recursion described in
		// eqn 5.
		// Note 1: We don't have to care about complex conjugates as our signals
		// are all real-valued
		// Note 2: j counts 1..order+1, i-j+1 counts order..0
		// Note 3: The first element of ar_coeffs* is always 1, which copies in
		// the reflection coefficient at the end of the new AR coefficient array
		// after the preceding coefficients
		std::swap(ar_coeffs_prev, ar_coeffs);

		for (int j = 1; j < i + 2; j++)
		{
			ar_coeffs[j] = ar_coeffs_prev[j] + reflect_coeff * ar_coeffs_prev[i - j + 1];
		}

		// Update the forward and backward prediction errors corresponding to
		// eqns 13 and 14.  We start with f_{M-1,k+1} and b_{M-1,k} and use them
		// to compute f_{M,k} and b_{M,k}
		auto fwd_pred_error_tmp = fwd_pred_error;

		// fwd_pred_error = fwd_pred_error + reflect_coeff * bwd_pred_error
		for (size_t k = 0; k < fwd_pred_error.size(); k++) {
			fwd_pred_error[k] += reflect_coeff * bwd_pred_error[k];
		}
		// bwd_pred_error = bwd_pred_error + reflect_coeff * fwd_pred_error_tmp
		for (size_t k = 0; k < bwd_pred_error.size(); k++) {
			bwd_pred_error[k] += reflect_coeff * fwd_pred_error_tmp[k];
		}

		// SNIP - we are now done with order M and advance. M-1 <- M

		// Compute DEN_{M} using the recursion from eqn 17.
		//
		// reflect_coeff = a_{M-1,M-1}      (we have advanced M)
		// den =  DEN_{M-1}                 (rhs)
		// bwd_pred_error = b_{M-1,N-M+1}   (we have advanced M)
		// fwd_pred_error = f_{M-1,k}       (we have advanced M)
		// den <- DEN_{M}                   (lhs)
		//

		auto q = 1 - pow(reflect_coeff, 2);
		den = q*den - pow(bwd_pred_error[-1], 2) - pow(fwd_pred_error[0], 2);

		// Shift up forward error.
		//
		// fwd_pred_error <- f_{M-1,k+1}
		// bwd_pred_error <- b_{M-1,k}
		//
		// N.B. We do this after computing the denominator using eqn 17 but
		// before using it in the numerator in eqn 15.

		auto new_size = fwd_pred_error.size() - 1;
		//fwd_pred_error = fwd_pred_error[1:]
		std::move(fwd_pred_error.begin() + 1, fwd_pred_error.end(), fwd_pred_error.begin());
		fwd_pred_error.resize(new_size);
		//bwd_pred_error = bwd_pred_error[:-1]
		bwd_pred_error.resize(new_size);
	}

	// Return an Eigen vector for the polynomial solver.
	Vector<double> result(ar_coeffs.size());
	std::copy(ar_coeffs.begin(), ar_coeffs.end(), result.data());

	return result;
#endif
}
#endif // 0

} // namespace phonometrica::speech
