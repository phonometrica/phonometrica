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
	m_data.resize(m_handle.frames() * m_handle.channels());
	double *ptr = m_data.data();
	m_handle.seek(0, SEEK_SET);

	if (load)
	{
		auto end = m_data.end();

		while (ptr < end)
		{
			auto count = m_handle.readf(ptr, BUFFER_SIZE);
			ptr += count * m_handle.channels();
			//emit bufferLoaded(progress);
		}
		m_handle.seek(0, SEEK_SET);
	}
}

Span<double> AudioData::get(intptr_t first_frame, intptr_t last_frame)
{
	if (last_frame < 0) last_frame = (intptr_t) m_handle.frames();
	assert(first_frame > 0);
	first_frame--;
	auto count = last_frame - first_frame;

	return { m_data.data() + first_frame, count };
}

} // namespace phonometrica