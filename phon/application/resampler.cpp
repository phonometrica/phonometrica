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
 * Created: 11/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cassert>
#include <cmath>
#include <phon/application/resampler.hpp>
#include <phon/error.hpp>

namespace phonometrica {

Resampler::Resampler(intptr_t in_rate, intptr_t out_rate, int quality, intptr_t nb_channels)
{
	int error;
    m_state = speex_resampler_init(nb_channels, in_rate, out_rate, quality, &error);

    if (error != 0 && m_state)
    {
        speex_resampler_destroy(m_state);
        m_state = nullptr;
        throw std::runtime_error("Could not initialize resampler");
    }

    ratio = double(in_rate) / out_rate;
}

Resampler::~Resampler()
{
	if (m_state) {
		speex_resampler_destroy(m_state);
	}
}

std::vector<float> Resampler::resample(const Span<float> &input)
{
	std::vector<float>output;
	resample(input, output);

	return output;
}

void Resampler::resample(const Span<float> &input, std::vector<float> &output)
{
	const int channel = 0;
	size_t count = floor(input.size() * ratio);
	output.resize(count);
	spx_uint32_t ilen = input.size();
	spx_uint32_t olen;
	speex_resampler_process_float(m_state, channel, input.data(), &ilen, output.data(), &olen);
	assert(olen <= input.size());
	output.resize(olen);
}
} // namespace phonometrica
