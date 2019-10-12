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
 * Purpose: audio resampler based on SPEEX.                                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_RESAMPLER_HPP
#define PHONOMETRICA_RESAMPLER_HPP

#include <cstdint>
#include <vector>
#include <phon/utils/span.hpp>
#include <phon/third_party/speex/speex_resampler.h>

namespace phonometrica {

class Resampler final
{
public:

	Resampler() = default;

	Resampler(intptr_t in_rate, intptr_t out_rate, int quality = 5, intptr_t nb_channels = 1);

	Resampler(const Resampler &) = delete;

	Resampler(Resampler &&) noexcept = default;

	~Resampler();

	std::vector<float> resample(const Span<float> &input);

	void resample(const Span<float> &input, std::vector<float> &output);

private:

    SpeexResamplerState *m_state = nullptr;

	double ratio = 0;
};

} // namespace phonometrica



#endif // PHONOMETRICA_RESAMPLER_HPP
