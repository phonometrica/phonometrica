/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 07/09/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: match object created that corresponds to a search constraint.                                             *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_MATCH_HPP
#define PHONOMETRICA_QUERY_MATCH_HPP

#include <memory>
#include <phon/application/annotation.hpp>

namespace phonometrica {


class QueryMatch;
using AutoQueryMatch = std::shared_ptr<QueryMatch>;

class QueryMatch
{
public:

	QueryMatch(const AutoAnnotation &annot, int layer, const AutoEvent &e) :
		m_annot(annot), m_event(e), m_layer_index(layer)
	{ }

	virtual ~QueryMatch() = default;

	double start_time() const;

	double end_time() const;

	bool operator<(const QueryMatch &other) const;

	const AutoAnnotation &annot() const { return m_annot; }

	const AutoEvent &event() const { return m_event; }

	QueryMatch *next() const { return m_next.get(); }

	int layer_index() const { return m_layer_index; }

	int position() const { return m_position; }

protected:

	// Annotation in which the match was found.
	AutoAnnotation m_annot;

	// Event where the match occurred.
	AutoEvent m_event;

	// Next match in a complex query (may be null).
	AutoQueryMatch m_next;

	// Index of the layer in which the match was found.
	int m_layer_index;

	// Position in the event, in case there are several matches (used for sorting).
	int m_position = 1;
};


struct MatchLessCompare
{
    bool operator()(const AutoQueryMatch &a, const AutoQueryMatch &b) const
    {
        return *a < *b;
    }
};

using QueryMatchSet = std::set<AutoQueryMatch, MatchLessCompare>;

//----------------------------------------------------------------------------------------------------------------------

class Concordance final : public QueryMatch
{
public:

	Concordance(const AutoAnnotation &annot, int layer, const AutoEvent &e, String left, String right) :
		QueryMatch(annot, layer, e), m_left(std::move(left)), m_right(std::move(right))
	{ }

	String left() const { return m_left; }

	String right() const { return m_right; }

private:

	// Left and right contexts of the concordance.
	String m_left, m_right;
};

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_MATCH_HPP