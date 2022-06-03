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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: parse Praat's TextGrid format.                                                                             *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PRAAT_HPP
#define PHONOMETRICA_PRAAT_HPP

#include <utility>
#include <phon/string.hpp>
#include <phon/file.hpp>

namespace phonometrica { namespace praat {

struct TierHeader
{
	String label;
	double xmin;
	double xmax;
	int size = 0;
	bool has_points;
};

struct Point
{
	double time;
	String text;
};

struct Interval
{
	double xmin;
	double xmax;
	String text;
};

// If a tier was found, sets the name of the tier and whether it is a point tier.
bool parse_tier_header(File &infile, const String &line, TierHeader &header);

bool parse_interval(File &infile, const String &line, Interval &interval);

bool parse_point(File &infile, const String &line, Point &point);

void open_textgrid(const String &tgd, const String &snd = String());

void open_sound(const String &path);

void open_interval(intptr_t tier, intptr_t interval, const String &textgrid, const String &sound);

}} // namespace phonometrica::praat

#endif // PHONOMETRICA_PRAAT_HPP
