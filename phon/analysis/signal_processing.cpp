/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 1997-2005  Kåre Sjölander <kare@speech.kth.se>                                                        *
 * Copyright (C) 1992-2019 Paul Boersma                                                                                *
 * Copyright (c) 1999-2010 by Kelly Fitz and Lippold Haken                                                             *
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
 * Created: 31/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: signal processing routines.                                                                                *
 *                                                                                                                     *
 * Note: This file contains code derived from the Snack Sound Toolkit. See file BSD.txt. The latest version can be     *
 * found at http://www.speech.kth.se/snack/.                                                                           *
 * The code for the Gaussian window and pre-emphasis is derived from Praat, see http://www.praat.org (GPL2+)           *
 * The code for zeroethOrderBessel and the Kaiser window are derived from the Loris Class library,                     *
 * see https://github.com/tractal/loris (GPL2+)                                                                        *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <numeric> // std::iota
#include <algorithm>
#include <SPTK.h>
#include <fftw3.h>
#include <phon/analysis/signal_processing.hpp>

namespace phonometrica {
namespace speech {


// ---------------------------------------------------------------------------
//  zeroethOrderBessel
// ---------------------------------------------------------------------------
//  Compute the zeroeth order modified Bessel function of the first kind
//  at x using the series expansion, used to compute the Kasier window
//  function.
//
static double zeroethOrderBessel(double x)
{
	constexpr double eps = 0.000001;

	//  initialize the series term for m=0 and the result
	double besselValue = 0;
	double term = 1;
	double m = 0;

	//  accumulate terms as long as they are significant
	while(term  > eps * besselValue)
	{
		besselValue += term;

		//  update the term
		++m;
		term *= (x*x) / (4*m*m);
	}

	return besselValue;
}


Array<double> create_window(intptr_t N, intptr_t fftlen, WindowType type)
{
    Array<double> result(fftlen, 0.0);
    intptr_t i;

    if (N > fftlen)
	    N = fftlen;

	double *win = result.data();

	switch (type)
    {
    case WindowType::Rectangular:
    {
        for (i = 0; i < N; i++)
            win[i] = (double) 1.0;
    }
        break;
    case WindowType::Hann:
    {
        for (i = 0; i < N; i++)
            win[i] = (double) (0.5 * (1.0 - cos(i * 2.0 * M_PI / (N - 1))));
    }
        break;
    case WindowType::Bartlett:
    {
        for (i = 0; i < N / 2; i++)
            win[i] = (double) (((2.0 * i) / (N - 1)));
        for (i = N / 2; i < N; i++)
            win[i] = (double) (2.0 * (1.0 - ((double) i / (N - 1))));
    }
        break;
    case WindowType::Blackman:
    {
        for (i = 0; i < N; i++)
            win[i] = (double) ((0.42 - 0.5 * cos(i * 2.0 * M_PI / (N - 1))
                                + 0.08 * cos(i * 4.0 * M_PI / (N - 1))));
    }
        break;
    case WindowType::Gaussian:
    {
	    double imid = 0.5 * (N + 1), edge = exp (-12.0);
	    for (i = 1; i <= N; i++) {
		    win[i-1] = (exp (-48.0 * (i - imid) * (i - imid) / (N + 1) / (N + 1)) - edge) / (1.0 - edge);
	    }
    }
	    break;
    case WindowType::Hamming:
    {
        for (i = 0; i < N; i++)
            win[i] = (double) (0.54 - 0.46 * cos(i * 2.0 * M_PI / (N - 1)));
    }
	case WindowType::Kaiser:
	{
		constexpr double beta = 0.5;
		//  Pre-compute the shared denominator in the Kaiser equation.
		const double oneOverDenom = 1.0 / zeroethOrderBessel(beta);
		N--;
		const double oneOverN = 1.0 / N;

		for (intptr_t n = 0; n <= N; ++n)
		{
			const double K = (2.0 * n * oneOverN) - 1.0;
			const double arg = sqrt(1.0 - (K * K));
			win[n] = zeroethOrderBessel(beta * arg) * oneOverDenom;
		}
	}
    }

    return result;
}

double get_intensity(std::span<double> frame, std::span<double> window)
{
	assert(frame.size() == window.size());
	constexpr double Iref = 4.0e-10;
	double power = 0.0;

	for (intptr_t i = 0; i < frame.size(); i++)
	{
		auto value = frame[i] * window[i];
		power += value * value;
	}
	auto avg_power = power / window.size();
	auto dB = 10 * log10(avg_power / Iref);

	return std::isfinite(dB) ? dB : 0.0;
}

Array<double>
get_intensity(std::span<double> input, int samplerate, intptr_t window_size, double time_step, WindowType type)
{
    auto window = create_window(window_size, window_size, type);
    std::span<double> win = window;
    auto frame_shift = int(time_step * samplerate);
    auto n = int(ceil(double(input.size() - frame_shift) / double(window_size - frame_shift)));
    auto data = input.begin();
    auto limit = input.end();
    assert(frame_shift < window_size);
    Array<double> output;
    output.reserve(n);

    while (data < limit)
    {
        auto frames_left = limit - data;
        if (frames_left < window_size) {
            break;
        }
        std::span<double> d(data, window_size);
        auto dB = get_intensity(d, win);
        output.push_back(dB);
        data += frame_shift;
    }

    return output;
}

// Credits: https://stackoverflow.com/a/12399290
template <typename T>
static std::vector<size_t> sort_indices(const std::vector<T> &v) {

	// initialize original index locations
	std::vector<size_t> idx(v.size());
	std::iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in v
	sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

	return idx;
}

// This function is a C++ translation of the function _lpc() in librosa (in audio.py).
// Copyright (C) librosa development team
// License: ISC License
static std::vector<double> lpc_burg(const Array<double> &x, int order)
{
    // This implementation follows the description of Burg's algorithm given in
    // section III of Marple's paper referenced in the following paper:
    //            A New Autoregressive Spectrum Analysis Algorithm.
	//           IEEE Transactions on Accoustics, Speech, and Signal Processing
	//           vol 28, no. 4, 1980.
    //
    // We use the Levinson-Durbin recursion to compute AR coefficients for each
    // increasing model order by using those from the last. We maintain two
    // arrays and then flip them each time we increase the model order so that
    // we may use all the coefficients from the previous order while we compute
    // those for the new one. These two arrays hold ar_coeffs for order M and
    // order M-1.  (Corresponding to a_{M,k} and a_{M-1,k} in eqn 5)
	std::vector<double> ar_coeffs(order + 1, 0.0);
    ar_coeffs[0] = 1.0;
    std::vector<double> ar_coeffs_prev(order + 1, 0.0);
    ar_coeffs_prev[0] = 1.0;

    // These two arrays hold the forward and backward prediction error. They
    // correspond to f_{M-1,k} and b_{M-1,k} in eqns 10, 11, 13 and 14 of
    // Marple. First they are used to compute the reflection coefficient at
    // order M from M-1 then are re-used as f_{M,k} and b_{M,k} for each
    // iteration of the below loop
    std::vector<double> fwd_pred_error(x.begin()+1, x.end());
    std::vector<double> bwd_pred_error(x.begin(), x.end()-1);

    // DEN_{M} from eqn 16 of Marple.
    auto den = std::inner_product(fwd_pred_error.begin(), fwd_pred_error.end(), fwd_pred_error.begin(), 0.0) +
    		std::inner_product(bwd_pred_error.begin(), bwd_pred_error.end(), bwd_pred_error.begin(), 0.0);

    for (int i = 0; i < order; i++)
	{
		if (den <= 0)
		{
#ifdef PHON_DEBUG
			PHON_LOG("Potential numerical error in LPC analysis: input ill-conditioned?\n");
#endif
			for (size_t k = 0; k < ar_coeffs.size(); k++) {
				ar_coeffs[k] = 0;
			}
			return ar_coeffs;
		}

		// Eqn 15 of Marple, with fwd_pred_error and bwd_pred_error
		// corresponding to f_{M-1,k+1} and b{M-1,k} and the result as a_{M,M}
		// reflect_coeff = dtype(-2) * np.dot(bwd_pred_error, fwd_pred_error) / dtype(den)
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
		for (int j = 1; j < i + 2; j++) {
			ar_coeffs[j] = ar_coeffs_prev[j] + reflect_coeff * ar_coeffs_prev[i - j + 1];
		}

		// Update the forward and backward prediction errors corresponding to
		// eqns 13 and 14.  We start with f_{M-1,k+1} and b_{M-1,k} and use them
		// to compute f_{M,k} and b_{M,k}
		auto fwd_pred_error_tmp = fwd_pred_error;
		for (size_t k = 0; k < fwd_pred_error.size(); k++) {
			fwd_pred_error[k] = fwd_pred_error[k] + reflect_coeff * bwd_pred_error[k];
		}
		for (size_t k = 0; k < bwd_pred_error.size(); k++) {
			bwd_pred_error[k] = bwd_pred_error[k] + reflect_coeff * fwd_pred_error_tmp[k];
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

		auto q = 1.0 - reflect_coeff * reflect_coeff;
		den = q * den - bwd_pred_error.back() * bwd_pred_error.back() - fwd_pred_error.front() * fwd_pred_error.front();

		// Shift up forward error.
		//
		// fwd_pred_error <- f_{M-1,k+1}
		// bwd_pred_error <- b_{M-1,k}
		//
		// N.B. We do this after computing the denominator using eqn 17 but
		// before using it in the numerator in eqn 15.
		fwd_pred_error.erase(fwd_pred_error.begin());
		bwd_pred_error.pop_back();
	}

    return ar_coeffs;
}

std::vector<double> get_lpc_coefficients(const Array<double> &x, int order)
{
	return lpc_burg(x, order);
}

// Formant estimation partly based on
// https://www.mathworks.com/help/signal/ug/formant-estimation-with-lpc-coefficients.html
bool get_formants(const std::vector<double> &lpc_coeffs, double Fs, std::vector<double> &freqs, std::vector<double> &bw)
{
	std::vector<std::complex<double>> root_candidates(lpc_coeffs.size(), std::complex<double>());
	int order = int(lpc_coeffs.size() - 1);
	bool ok = root_pol(const_cast<double*>(lpc_coeffs.data()), order, (complex*)root_candidates.data(), 1, 1.0e-14, 1000);
	if (!ok) return false;
	std::vector<double> angz;
	std::vector<std::complex<double>> roots;

	for (auto z : root_candidates)
	{
		if (z.imag() >= 0)
		{
			// Make sure all roots lie within the unit circle.
			if (std::abs(z) > 1) {
				z = std::complex<double>(1.0, 0.0) / std::conj(z);
			}
			roots.push_back(z);
		}
	}

	for (auto z : roots)
	{
		double angle = atan2(z.imag(), z.real());
		double f = angle * (Fs / M_2PI);
		freqs.push_back(f);
	}

	auto indices = sort_indices(freqs);
	std::sort(freqs.begin(), freqs.end());

	for (auto i : indices)
	{
		// From eqn (2) in:
		//     Snell, Roy C & Fausto Milinazzo. 1993. Formant location from LPC analysis data.
		//     IEEE transactions on Speech and Audio Processing 1(2). 129–134.
		double b = -log(std::abs(roots[i])) * (Fs/M_PI);
		bw.push_back(b);
	}

	return true;
}

// Adapted from Praat's pre-emphasis routine in Sound_to_Formant.cpp
// Copyright (C) 1992-2008,2010-2012,2014-2020 Paul Boersma
// License: GPL 2 or later

void pre_emphasis(Array<double> &data, double Fs, double threshold)
{
	auto x = data.data();
	double alpha = exp(-2 * M_PI * threshold * (1.0 / Fs));
	auto len = data.size();

	for (auto i = len; i > 1; i--) {
		x[i] -= alpha * x[i-1];
	}
}

Array<std::complex<double>> specgram(const Array<double> &data, int nfft, intptr_t noverlap, intptr_t window_size, WindowType window_type)
{
	if (window_type == WindowType::Gaussian) {
		window_size *= 2;
	}

	auto nrow = nfft / 2 + 1;
	auto ncol = (data.size() - noverlap) / (window_size - noverlap);

	Array<std::complex<double>> result(nrow, ncol, {0.0, 0.0});
	std::vector<double> input(nfft, 0.0);
	std::vector<std::complex<double>> output(nfft, std::complex<double>(0, 0));
	fftw_plan plan = fftw_plan_dft_r2c_1d(nfft, input.data(), (fftw_complex*)output.data(), FFTW_ESTIMATE);
	auto len = data.size();
	auto win = create_window(window_size, nfft, window_type);
	intptr_t j = 1;

	for (intptr_t k = 0; k + window_size < len; k += noverlap)
	{
		auto sample = data.begin() + k;
		auto buffer = input.begin();

		for (intptr_t n = 1; n <= window_size; n++) {
			*buffer++ = *sample++ * win[n];
		}
		for (intptr_t n = window_size + 1; n <= nfft; n++) {
			*buffer++ = 0.0;
		}
		fftw_execute(plan);
		auto z = output.begin();

		for (intptr_t i = 1; i <= nrow; i++) {
			result(i,j) = *z++;
		}
		j++;
	}
	assert(j-1 == result.ncol());
	fftw_free(plan);

	return result;
}


//---------------------------------------------------------------------------------------------------------------------

FFT::FFT(intptr_t length) : nfft(length), input(length, 0.0), output(length, std::complex<double>(0, 0))
{
	impl = fftw_plan_dft_r2c_1d((int)length, input.data(), (fftw_complex*)output.data(), FFTW_ESTIMATE);
}

FFT::~FFT()
{
	fftw_free(reinterpret_cast<fftw_plan>(impl));
}

Array<std::complex<double>> &FFT::process(const Array<double> &data)
{
	auto len = (std::min<intptr_t>)(data.size(), nfft);
	std::copy(data.begin(), data.begin()+len, input.begin());
	auto sample = data.begin();
	auto buffer = input.begin();

	for (intptr_t n = 1; n <= len; n++) {
		*buffer++ = *sample++;
	}
	// Pad with zeros if the data is shorter than nfft.
	for (intptr_t n = len + 1; n <= nfft; n++) {
		*buffer++ = 0.0;
	}
	fftw_execute(reinterpret_cast<fftw_plan>(impl));

	return output;
}
}} // namespace phonometrica::speech
