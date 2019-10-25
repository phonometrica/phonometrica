/***********************************************************************************************************************
 *                                                                                                                     *
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