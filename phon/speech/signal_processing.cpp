/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 1997-2005  Kåre Sjölander <kare@speech.kth.se>                                                        *
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
 * Created: 31/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: signal processing routines.                                                                                *
 *                                                                                                                     *
 * Note: This file contains code derived from the Snack Sound Toolkit. See file BSD.txt. The latest version can be     *
 * found at http://www.speech.kth.se/snack/                                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <numeric> // std::iota
#include <algorithm>
#include <SPTK.h>
#include <phon/speech/signal_processing.hpp>

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
    	const double sigma = 0.45 * (N - 1.0) / 2;
	    const double sig2 = 2 * sigma * sigma;

        for (i = 0; i < N; i++)
        {
        	int n = i - (N-1) / 2;
        	win[i] = exp((-n * n) / sig2);
        }
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

Array<double>
get_intensity(const Span<double> &input, int samplerate, intptr_t window_size, double time_step, WindowType type)
{
    auto window = create_window(window_size, window_size, type);
    auto win = window.data();
    auto frame_shift = int(time_step * samplerate);
//    const int n = int((double(input.size() - window_size))/frame_shift)+1;
    auto n = int(ceil(double(input.size() - frame_shift) / double(window_size - frame_shift)));
    auto data = input.begin();
    auto limit = input.end();
    assert(frame_shift < window_size);
    Array<double> output(n);

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
        auto d = data;
        double power = 0.0;
        for (intptr_t i = 0; i < window_size; i++)
        {
            assert(d < limit);
            double sample = *d++;
            assert(std::isfinite(sample));
            auto value = sample * win[i];
            power += value * value;
        }

        auto avg_power = power / window_size;
        assert(std::isfinite(avg_power));
        constexpr double Iref = 4.0e-10;
        auto dB = 10 * log10(avg_power / Iref);
        output.append(dB);
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

// Formant estimatation adapted from
// https://www.mathworks.com/help/signal/ug/formant-estimation-with-lpc-coefficients.html

std::vector<double> get_lpc_coefficients(const Span<double> &frame, int npole)
{
	std::vector<double> coeff(npole+1, 0.0);
	lpc(frame.data(), frame.size(), coeff.data(), npole, 0.000001);

	return coeff;
}

std::pair<std::vector<double>, std::vector<double>> get_formants(const Span<double> &lpc_coeffs, double Fs)
{
	std::vector<std::complex<double>> roots(lpc_coeffs.size(), std::complex<double>());
	root_pol(lpc_coeffs.data(), lpc_coeffs.size(), (complex*)roots.data(), 0, 1.0e-14, 1000);
	std::vector<double> angz;

	std::vector<std::complex<double>> tmp;

	for (auto z : roots)
	{
		if (z.imag() >= 0) tmp.push_back(z);
	}
	roots = std::move(tmp);

	std::vector<double> freqs;

	for (auto z : roots)
	{
		double angle = atan2(z.imag(), z.real());
		double f = angle * (Fs / M_2PI);
		freqs.push_back(f);
	}

	auto indices = sort_indices(freqs);
	std::sort(freqs.begin(), freqs.end());
	std::vector<double> bw;

	for (auto i : indices)
	{
		double b = -0.5*(Fs/M_2PI)*log(std::abs(roots[i]));
		bw.push_back(b);
	}

	return { freqs, bw };
}

//void root_pol(double *a, const int odr, complex * x, const int a_zero,
//              const double eps, const int itrat)



}} // namespace phonometrica::speech
