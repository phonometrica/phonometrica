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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/audio_data.hpp>
#include "audio_data.hpp"


namespace phonometrica {

static const int BUFFER_SIZE = 2048;

AudioData::AudioData(const SndfileHandle &h) :
	AudioData(h, true)
{

}

AudioData::AudioData(const SndfileHandle &h, bool load) :
    m_handle(h)
{
	m_handle.seek(0, SEEK_SET);

	if (load)
	{
		m_data.resize(m_handle.frames() * m_handle.channels());
		auto ptr = m_data.begin();
		auto end = m_data.end();

		while (ptr < end)
		{
			auto count = m_handle.readf(&*ptr, BUFFER_SIZE);
			ptr += count * m_handle.channels();
			//emit bufferLoaded(progress);
		}
		m_handle.seek(0, SEEK_SET);
	}
}

Span<double> AudioData::get(intptr_t first_frame, intptr_t last_frame)
{
	if (last_frame < 0) last_frame = (intptr_t) m_handle.frames() - 1;
	assert(first_frame >= 0);
	auto count = last_frame - first_frame;

	return { m_data.data() + first_frame, count };
}

std::vector<double> AudioData::copy(intptr_t first_frame, intptr_t last_frame)
{
	if (last_frame < 0) last_frame = (intptr_t) m_handle.frames() - 1;
	assert(first_frame >= 0);
	auto from = m_data.data() + first_frame;
	auto to = m_data.data() + last_frame;

	return std::vector<double>(from, to);
}

std::vector<float> AudioData::read(intptr_t first_frame, intptr_t last_frame)
{
	if (last_frame < 0) last_frame = (intptr_t) m_handle.frames() - 1;
	assert(first_frame >= 0);
	auto count = last_frame - first_frame;
	std::vector<float> result(count * m_handle.channels(), 0.0);

	auto ptr = result.data();
	auto end = result.data() + result.size();
	seek(first_frame);

	while (ptr < end)
	{
		int nframe = (std::min)(BUFFER_SIZE, int(end-ptr));
		auto N = m_handle.readf(ptr, nframe);
		ptr += N * m_handle.channels();
	}
	seek(0);

	return result;
}

int AudioData::read(double *buffer, int count)
{
	return m_handle.readf(buffer, count);
}

void AudioData::seek(size_t pos)
{
	m_handle.seek(pos, SEEK_SET);
}

} // namespace phonometrica