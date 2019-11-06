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
	assert(input_rate > 0);
	assert(output_rate > 0);
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
