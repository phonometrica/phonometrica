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

std::vector<double> resample(Span<double> input, double input_rate, double output_rate)
{
	const int BUFFER_SIZE = 1024;
	double buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(double) * BUFFER_SIZE);
	Resampler resampler(input_rate, output_rate, BUFFER_SIZE);
	intptr_t ol = double(input.size()) * output_rate / input_rate;
	std::vector<double> output(ol, 0.0);
	auto it = input.begin();
	double *out = nullptr;
	double *data = output.data();

	while (ol > 0)
	{
		intptr_t count = (it + BUFFER_SIZE > input.end()) ? intptr_t(input.end() - it) : BUFFER_SIZE;
		Span<double> chunk;

		if (count == 0)
		{
			chunk = Span<double>(buffer, BUFFER_SIZE);
		}
		else
		{
			chunk = Span<double>(it, count);
		}
		auto len = resampler.process(chunk.data(), chunk.size(), out);
		if (len > ol) len = ol;
		std::copy(out, out+len, data);
		it += count;
		data += len;
		ol -= len;
	}

	return output;
}

} // namespace phonometrica
