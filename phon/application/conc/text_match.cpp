/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
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
 * Created: 01/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/conc/text_match.hpp>

namespace phonometrica {

double TextMatch::start_time() const
{
	const Match *match = this;
	auto value = (std::numeric_limits<double>::max)();

	while (match)
	{
		auto t = match->event()->start_time();
		if (t < value) value = t;
		match = match->next();
	}

	return value;
}

double TextMatch::end_time() const
{
	const Match *match = this;
	double value = 0;

	while (match)
	{
		auto t = match->event()->end_time();
		if (t > value) value = t;
		match = match->next();
	}

	return value;
}

bool TextMatch::operator<(const TextMatch &other) const
{
	int cmp;

	// First compare file names.
	cmp = this->annotation()->path().compare(other.annotation()->path());

	if (cmp < 0) {
		return true;
	}
	else if (cmp > 0) {
		return false;
	}

	// Next, compare tier in the file
	cmp = (this->layer_index() - other.layer_index());

	if (cmp < 0) {
		return true;
	}
	else if (cmp > 0) {
		return false;
	}

	// Next, Compare temporal position
	double delta = (this->start_time() - other.start_time());

	if (delta < 0) {
		return true;
	}
	else if (delta > 0) {
		return false;
	}

	// Finally compare position in the event.
	return this->offset() < other.offset();
}

size_t TextMatch::hash() const
{
	return annotation()->path().hash() + std::hash<int>{}(layer_index()) + text().hash() + std::hash<int>{}(offset());
}
} // namespace phonometrica
