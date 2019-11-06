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
#define USE_SPTK 1

#if USE_SPTK
#include <SPTK.h>
#else
#include <numeric>
#include <vector>
#endif

#include <phon/utils/matrix.hpp>

namespace phonometrica::speech {

Vector<double> get_lpc_coefficients(const Span<double> &frame, int order)
{
	assert(order > 1);

#if USE_SPTK
	Vector<double> coeff(order + 1);
	lpc(frame.data(), frame.size(), coeff.data(), order, 0.000001);

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
	std::vector<double> fwd_pred_error(frame.begin() + 1, frame.end());
	std::vector<double> bwd_pred_error(frame.begin(), frame.end() - 1);

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

} // namespace phonometrica::speech
