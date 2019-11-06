/***********************************************************************************************************************
 *                                                                                                                     *
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
 * Created: 19/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Audio data.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_AUDIO_DATA_HPP
#define PHONOMETRICA_AUDIO_DATA_HPP

#if PHON_WINDOWS
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif

#include <cmath>
#include <memory>
#include <sndfile.hh>
#include <phon/array.hpp>
#include <phon/utils/span.hpp>

namespace phonometrica {

class AudioData final
{
public:

    explicit AudioData(const SndfileHandle &h);

	AudioData(const SndfileHandle &h, bool load);

    const double *data() const { return m_data.data(); }

    double *data() { return m_data.data(); }

    Span<double> get(intptr_t first_frame = 0, intptr_t last_frame = -1);

	std::vector<double> copy(intptr_t first_frame = 0, intptr_t last_frame = -1);

	std::vector<float> read(intptr_t first_frame, intptr_t last_frame);

	int read(double *buffer, int count);

	void seek(size_t pos);

    double sample(intptr_t i) const { return m_data[i]; }

    intptr_t size() const { return m_handle.frames() / m_handle.channels(); }

    int channels() const { return m_handle.channels(); }

    double max_value() const { return *std::max_element(m_data.begin(), m_data.end()); }

    double min_value() const { return *std::min_element(m_data.begin(), m_data.end()); }

    int sample_rate() const { return m_handle.samplerate(); }

    double frame_to_time(intptr_t index) const { return double(--index) / sample_rate(); }

    intptr_t time_to_frame(double time) const
    {
        auto s = (intptr_t) round(time * sample_rate());
        return std::min<intptr_t>(s, m_handle.frames() - 1);
    }

    double duration() const { return (double)m_handle.frames() / sample_rate(); }

    SndfileHandle handle() const { return m_handle; }

private:

    std::vector<double> m_data;

    SndfileHandle m_handle;
};

} // namespace phonometrica

#endif // PHONOMETRICA_AUDIO_DATA_HPP
