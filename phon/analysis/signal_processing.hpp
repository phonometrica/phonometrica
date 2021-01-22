/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 1997-2005  Kåre Sjölander <kare@speech.kth.se>                                                        *
 * Copyright (C) 1992-2019 Paul Boersma                                                                                *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * The code for the Gaussian window is derived from Praat, see http://www.praat.org.                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SIGNAL_PROCESSING_HPP
#define PHONOMETRICA_SIGNAL_PROCESSING_HPP

#include <cmath>
#include <complex>
#include <vector>
#include <phon/array.hpp>
#include <phon/utils/span.hpp>
#include <phon/utils/matrix.hpp>

namespace phonometrica { namespace speech {

enum class WindowType
{
    Bartlett,
    Blackman,
    Gaussian,
    Hamming,
    Hann,
    Rectangular
};


Array<double> create_window(intptr_t N, intptr_t fftlen, WindowType type);

// Get intensity for a frame.
double get_intensity(std::span<double> frame, std::span<double> window);

std::vector<double> get_intensity(std::span<double> input, int samplerate, intptr_t window_size, double time_step, WindowType type = WindowType::Hamming);


template<typename Container>
void pre_emphasis(Container &data, double Fs, double threshold)
{
	using T = typename Container::value_type;
	T *x = data.data();
	double alpha = exp(-2 * M_PI * threshold * (1.0 / Fs));

	x[0] = x[0] * (1.0 - alpha);

	for (intptr_t k = 1; k < data.size(); k++)
	{
		x[k] = x[k] - alpha * x[k-1];
	}
}

// Calculate LPC coefficients from a speech frame.
Array<double> get_lpc_coefficients(const Array<double> &frame, int npole);

// Get formant frequencies and bandwidths from a set of LPC coefficients.
bool get_formants(const Array<double> &lpc_coeffs, double Fs, std::vector<double> &freqs, std::vector<double> &bw);

}} // namespace phonometrica::speech

#endif // PHONOMETRICA_SIGNAL_PROCESSING_HPP
