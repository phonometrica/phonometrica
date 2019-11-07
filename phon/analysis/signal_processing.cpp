/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 1997-2005  Kåre Sjölander <kare@speech.kth.se>                                                        *
 * Copyright (C) 1992-2019 Paul Boersma                                                                                *
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
 * Created: 31/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: signal processing routines.                                                                                *
 *                                                                                                                     *
 * Note: This file contains code derived from the Snack Sound Toolkit. See file BSD.txt. The latest version can be     *
 * found at http://www.speech.kth.se/snack/.                                                                           *
 * The code for the Gaussian window is derived from Praat, see http://www.praat.org.                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <numeric> // std::iota
#include <algorithm>
#include <SPTK.h>
#include <phon/analysis/signal_processing.hpp>
#include <phon/third_party/rpolyplusplus/find_polynomial_roots_jenkins_traub.h>
#include <QDebug>

namespace phonometrica {
namespace speech {

Array<double> create_window(intptr_t N, intptr_t fftlen, WindowType type)
{
    Array<double> result;
    intptr_t i;

    if (N > fftlen)
	    N = fftlen;

	result.resize(fftlen);
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
//    	const double sigma = 0.45 * (N - 1.0) / 2;
//	    const double sig2 = 2 * sigma * sigma;
//
//        for (i = 0; i < N; i++)
//        {
//        	int n = i - (N-1) / 2;
//        	win[i] = exp((-n * n) / sig2);
//        }
    }
	    break;
    case WindowType::Hamming:
    {
        for (i = 0; i < N; i++)
            win[i] = (double) (0.54 - 0.46 * cos(i * 2.0 * M_PI / (N - 1)));
    }
    }

    for (i = N; i < fftlen; i++)
        win[i] = 0.0;

    return result;
}

double get_intensity(Span<double> frame, Span<double> window)
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

	return 10 * log10(avg_power / Iref);
}

std::vector<double>
get_intensity(Span<double> input, int samplerate, intptr_t window_size, double time_step, WindowType type)
{
    auto window = create_window(window_size, window_size, type);
    Span<double> win = window;
    auto frame_shift = int(time_step * samplerate);
    auto n = int(ceil(double(input.size() - frame_shift) / double(window_size - frame_shift)));
    auto data = input.begin();
    auto limit = input.end();
    assert(frame_shift < window_size);
    std::vector<double> output;
    output.reserve(n);

    while (data < limit)
    {
        auto frames_left = limit - data;
        if (frames_left < window_size)
        {
            break;
//            window = create_window(frames_left, frames_left, type);
//            win = window.data();
//            window_size = frames_left;
        }
        Span<double> d(data, window_size);
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

// Formant estimation based on
// https://www.mathworks.com/help/signal/ug/formant-estimation-with-lpc-coefficients.html
bool get_formants(const Vector<double> &lpc_coeffs, double Fs, std::vector<double> &freqs, std::vector<double> &bw)
{
#if 1
	std::vector<std::complex<double>> roots(lpc_coeffs.size(), std::complex<double>());
	int order = lpc_coeffs.size() - 1;
	bool ok = root_pol(const_cast<double*>(lpc_coeffs.data()), order, (complex*)roots.data(), 1, 1.0e-14, 1000);
	if (!ok) return false;
	std::vector<double> angz;

	std::vector<std::complex<double>> tmp;

	for (auto z : roots)
	{
		if (z.imag() >= 0) tmp.push_back(z);
	}
	roots = std::move(tmp);
#else
	int order = lpc_coeffs.size() - 1;
	Eigen::VectorXd real_roots(order), complex_roots(order);
	bool ok = rpoly_plus_plus::FindPolynomialRootsJenkinsTraub(lpc_coeffs, &real_roots, &complex_roots);
	if (!ok) return false;
	std::vector<double> angz;
	std::vector<std::complex<double>> roots;

	for (int i = 0; i < complex_roots.size(); i++)
	{
		if (complex_roots[i] >= 0)
		{
			std::complex<double> z = { real_roots[i], complex_roots[i] };
			// Reflect roots that lie outside the unit circle inside it.
			if (std::abs(z) > 1) {
				z = std::complex<double>(1.0) / std::conj(z);
			}
			roots.emplace_back(z);
		}
	}
#endif // 0

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
		double b = -0.5 * (Fs/M_2PI) * log(std::abs(roots[i]));
		bw.push_back(b);
	}

	return true;
}

}} // namespace phonometrica::speech
