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
 * Purpose: Match is the base class for all matches: it represents a line of concordance. For simple queries with      *
 * a single constraint, there is a single match; for complex queries with two or more constraints, the match is        *
 * represented as a linked list of matches. Match only contains information about the location of the match in a file, *
 * but not about the file itself: this information is stored in derived classes (see TextMatch).                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_MATCH_HPP
#define PHONOMETRICA_MATCH_HPP

#include <phon/application/agraph.hpp>

namespace phonometrica {

class Match
{
public:

	Match(const AutoEvent &e, String target, int layer, int offset);

	const AutoEvent &event() const { return m_event; }

	Match *next() const { return m_next.get(); }

	int layer_index() const { return m_layer_index; }

	int offset() const { return m_offset; }

	String text() const { return m_target; }

	void append_match(std::unique_ptr<Match> m);

protected:

	// Event where the match occurred.
	AutoEvent m_event;

	// Matched text.
	String m_target;

	// Index of the layer in which the match was found.
	int m_layer_index;

	// Offset where the match occurred in the event.
	int m_offset;

	// Next match in a complex query (may be null).
	std::unique_ptr<Match> m_next;
};


} // namespace phonometrica



#endif // PHONOMETRICA_MATCH_HPP
