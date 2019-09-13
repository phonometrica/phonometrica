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
#include <unordered_set>
#include <phon/application/annotation.hpp>

namespace phonometrica {


class QueryMatch;
using AutoQueryMatch = std::shared_ptr<QueryMatch>;

class QueryMatch
{
public:

	QueryMatch(const AutoAnnotation &annot, int layer, const AutoEvent &e, const String &text, int position = 1) :
		m_annot(annot), m_event(e), m_text(text), m_layer_index(layer), m_position(position)
	{ }

	virtual ~QueryMatch() = default;

	double start_time() const;

	double end_time() const;

	bool operator<(const QueryMatch &other) const;

	const AutoAnnotation &annotation() const { return m_annot; }

	const AutoEvent &event() const { return m_event; }

	QueryMatch *next() const { return m_next.get(); }

	int layer_index() const { return m_layer_index; }

	int position() const { return m_position; }

	String text() const { return m_text; }

	size_t hash() const;

protected:

	// Annotation in which the match was found.
	AutoAnnotation m_annot;

	// Event where the match occurred.
	AutoEvent m_event;

	// Matched text.
	String m_text;

	// Next match in a complex query (may be null).
	AutoQueryMatch m_next;

	// Index of the layer in which the match was found.
	int m_layer_index;

	// Position in the event, in case there are several matches (used for sorting).
	int m_position;
};


struct MatchLessCompare
{
    bool operator()(const AutoQueryMatch &a, const AutoQueryMatch &b) const
    {
        return *a < *b;
    }
};

struct MatchEqualCompare
{
	bool operator()(const AutoQueryMatch &a, const AutoQueryMatch &b) const
	{
		return (a->annotation()->path() == b->annotation()->path()) &&
				(a->layer_index() == b->layer_index()) && (a->text() == b->text()) && (a->position() == b->position());
	}
};

struct MatchHash
{
	size_t operator()(const AutoQueryMatch &a) const
	{
		return a->hash();
	}
};

using QueryMatchSet = std::set<AutoQueryMatch, MatchLessCompare>;
//using QueryMatchSet = std::unordered_set<AutoQueryMatch, MatchHash, MatchEqualCompare>;
using QueryMatchList = Array<AutoQueryMatch>;

//----------------------------------------------------------------------------------------------------------------------

class Concordance final : public QueryMatch
{
public:

	Concordance(const AutoAnnotation &annot, int layer, const AutoEvent &e, const String &text, int pos, String left, String right) :
		QueryMatch(annot, layer, e, text, pos), m_left(std::move(left)), m_right(std::move(right))
	{ }

	String left() const { return m_left; }

	String right() const { return m_right; }

private:

	// Left and right contexts of the concordance.
	String m_left, m_right;
};

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_MATCH_HPP
