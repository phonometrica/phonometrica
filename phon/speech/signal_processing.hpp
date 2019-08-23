/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 1997-2005 Kare Sjolander <kare@speech.kth.se>                                                        *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 31/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 * Note: This file contains code derived from the Snack Sound Toolkit. The latest version can be found at             *
 * http://www.speech.kth.se/snack/                                                                                    *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_SIGNAL_PROCESSING_HPP
#define PHONOMETRICA_SIGNAL_PROCESSING_HPP

#include <cmath>
#include <phon/array.hpp>

namespace phonometrica { namespace speech {

enum class WindowType
{
    Bartlett,
    Blackman,
    Hamming,
    Hann,
    Rectangular
};


Array<double> create_window(intptr_t winlen, intptr_t fftlen, WindowType type);

Array<double> get_intensity(const Array<double> &input, int samplerate, intptr_t window_size, double time_step, WindowType type = WindowType::Hamming);

}} // namespace phonometrica::speech

#endif // PHONOMETRICA_SIGNAL_PROCESSING_HPP
