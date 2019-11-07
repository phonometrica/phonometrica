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
 * Created: 07/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/search/query_match.hpp>

namespace phonometrica {

double QueryMatch::start_time() const
{
	auto match = this;
	auto value = (std::numeric_limits<double>::max)();

	while (match)
	{
		auto t = match->m_event->start_time();
		if (t < value) value = t;
		match = match->m_next.get();
	}

	return value;
}

double QueryMatch::end_time() const
{
	auto match = this;
	double value = 0;

	while (match)
	{
		auto t = match->m_event->end_time();
		if (t > value) value = t;
		match = match->m_next.get();
	}

	return value;
}

bool QueryMatch::operator<(const QueryMatch &other) const
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
	return this->position() < other.position();
}

size_t QueryMatch::hash() const
{
	return annotation()->path().hash() + std::hash<int>{}(layer_index()) + text().hash() + std::hash<int>{}(position());
}

//----------------------------------------------------------------------------------------------------------------------


AutoBookmark Concordance::to_bookmark(const String &title, const String &notes) const
{

	auto b = std::make_shared<AnnotationStamp>(nullptr, title, m_annot, layer_index(), start_time(),
			end_time(), text(), left(), right());
	b->set_notes(notes);

	return b;
}

//----------------------------------------------------------------------------------------------------------------------

AutoBookmark Measurement::to_bookmark(const String &title, const String &notes) const
{
	// TODO: improve bookmarking for phonetic measurements.
	auto b = std::make_shared<AnnotationStamp>(nullptr, title, m_annot, layer_index(), start_time(),
	                                           end_time(), text(), String(), String());
	b->set_notes(notes);

	return b;
}

String Measurement::left() const
{
	return labels[1];
}

String Measurement::right() const
{
	return labels[2];
}

String Measurement::get_label(intptr_t i) const
{
	return labels[i];
}

String FormantMeasurement::get_field(intptr_t j) const
{
	static String undef("undefined");
	double f = m_formants(1, j);

	return std::isnan(f) ? undef : String::format("%.2f", f);
}
} // namespace phonometrica
