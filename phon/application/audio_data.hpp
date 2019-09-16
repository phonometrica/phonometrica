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

namespace phonometrica {

using sample_t = int;


class AudioData final
{
public:

    explicit AudioData(const SndfileHandle &h);

	AudioData(const SndfileHandle &h, bool load);

    const sample_t *data() const { return m_data.data(); }

    sample_t *data() { return m_data.data(); }

    Array<double> real_data(intptr_t first_frame = 1, intptr_t last_frame = -1);

    sample_t sample(intptr_t i) const { return m_data[i]; }

    intptr_t size() const { return m_handle.frames(); }

    int channels() const { return m_handle.channels(); }

    sample_t max_value() const { return *std::max_element(m_data.begin(), m_data.end()); }

    sample_t min_value() const { return *std::min_element(m_data.begin(), m_data.end()); }

    int sample_rate() const { return m_handle.samplerate(); }

    double frame_to_time(intptr_t index) const { return double(--index) / sample_rate(); }

    intptr_t time_to_frame(double time) const
    {
        auto s = (intptr_t) round(time * sample_rate() + 1);
        return std::min<intptr_t>(s, m_handle.frames());
    }

    double duration() const { return (double)m_handle.frames() / sample_rate(); }

    SndfileHandle handle() const { return m_handle; }

private:

    Array<sample_t> m_data;

    SndfileHandle m_handle;
};

} // namespace phonometrica

#endif // PHONOMETRICA_AUDIO_DATA_HPP
