/**********************************************************************************************************************
 *                                                                                                                    *
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
 * Created: 19/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <phon/application/audio_data.hpp>
#include "audio_data.hpp"


namespace phonometrica {

static const int BUFFER_SIZE = 2048;

AudioData::AudioData(const SndfileHandle &h) :
    m_handle(h)
{
    m_data.resize(m_handle.frames() * m_handle.channels());
    sample_t *ptr = m_data.data();
    auto end = m_data.end();
    m_handle.seek(0, SEEK_SET);

    while (ptr < end)
    {
        auto count = m_handle.readf(ptr, BUFFER_SIZE);
        ptr += count * m_handle.channels();
        //emit bufferLoaded(progress);
    }
    m_handle.seek(0, SEEK_SET);
}

Array<double> AudioData::real_data(intptr_t first_frame, intptr_t last_frame)
{
    Array<double> result;
    if (last_frame < 0) last_frame = (intptr_t) m_handle.frames();
    assert(first_frame > 0);
    first_frame--;
    auto count = last_frame - first_frame;

    result.resize(count * m_handle.channels());
    auto *ptr = result.data();
    auto end = result.end();
    m_handle.seek(first_frame, SEEK_SET);

    while (ptr < end)
    {
        int nframe = (std::min)(BUFFER_SIZE, int(end-ptr));
        auto N = m_handle.readf(ptr, nframe);
        ptr += N * m_handle.channels();
    }
    m_handle.seek(0, SEEK_SET);

    return result;
}

} // namespace phonometrica