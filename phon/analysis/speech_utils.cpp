/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne <jeychenne@gmail.com>                                                       *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 04/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/analysis/speech_utils.hpp>

namespace phonometrica::speech {

// Credits: https://helloacm.com/how-to-implement-and-unit-test-linespace-algorithm-in-c/
std::vector<double> linspace(double from, double to, int num, bool include_boundaries)
{
	std::vector<double> points;
	// Exclude end points but add 2 points computed *inside* the window.
	if (!include_boundaries) num += 2;

	double length = (to - from) / (num - 1);

	if (include_boundaries) {
		points.push_back(from);
	}
	for (int i = 1; i < num - 1; i ++) {
		points.push_back(from + i * length);
	}
	if (include_boundaries) {
		points.push_back(to);
	}

	return points;
}

std::vector<double> get_time_points(double from, double to, double step, bool include_boundaries)
{
	std::vector<double> points;
	points.reserve((to - from) / step);
	if (include_boundaries) points.push_back(from);
	double t = from + step;
	while (t < to)
	{
		points.push_back(t);
		t += step;
	}
	if (include_boundaries) points.push_back(to);

	return points;
}

double hertz_to_bark(double hz)
{
	double z = ((26.81 * hz) / (1960 + hz)) - 0.53;
	if (z < 2) z = z + 0.15 * (2 - z);
	else if (z > 20.1) z = z + 0.22 * (z - 20.1);

	return z;
}

double bark_to_hertz(double z)
{
	if (z < 2) z = (z - 0.3) / 0.85;
	else if (z > 20.1) z = (z + 4.422) / 1.22;
	double hz = 1960 * ((z + 0.53) / (26.28 - z));

	return hz;
}

double hertz_to_erb(double hz)
{
	constexpr double A = 1000 * 2.302585093 / (24.7 * 4.37);
	double erb = A * log10(1 + 0.00437 * hz);

	return erb;
}

double erb_to_hertz(double erb)
{
	constexpr double A = 1000 * 2.302585093 / (24.7 * 4.37);
	double hz = (10 * (erb / A) - 1) / 0.00437;

	return hz;
}

double hertz_to_mel(double hz)
{
	return 2595 * log10(1 + hz / 700);
}

double mel_to_hertz(double mel)
{
	return 700 * (pow(10, mel / 2595) - 1);
}

double hertz_to_semitones(double f0, double ref)
{
	return 12 * log2(f0 / ref);
}

double semitones_to_hertz(double st, double ref)
{
	return ref * pow(2, st / 12);
}

} // namespace phonometrica::speech
