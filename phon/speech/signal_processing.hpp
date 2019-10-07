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

#ifndef PHONOMETRICA_SIGNAL_PROCESSING_HPP
#define PHONOMETRICA_SIGNAL_PROCESSING_HPP

#include <cmath>
#include <phon/array.hpp>

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


Array<double> create_window(intptr_t winlen, intptr_t fftlen, WindowType type);

Array<double> get_intensity(const Array<double> &input, int samplerate, intptr_t window_size, double time_step, WindowType type = WindowType::Hamming);


template<typename T>
void pre_emphasis(Array<T> &data, double alpha = 0.97)
{
	T *x = data.data();

	x[0] = x[0] * (1.0 - alpha);

	for (intptr_t k = 1; k < data.size(); k++)
	{
		x[k] = x[k] - alpha * x[k-1];
	}
}

}} // namespace phonometrica::speech

#endif // PHONOMETRICA_SIGNAL_PROCESSING_HPP
