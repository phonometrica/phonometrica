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
 * Created: 07/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: match object created that corresponds to a search constraint.                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_MATCH_HPP
#define PHONOMETRICA_QUERY_MATCH_HPP

#include <memory>
#include <set>
#include <phon/application/annotation.hpp>
#include <phon/application/bookmark.hpp>

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

	virtual AutoBookmark to_bookmark(const String &title, const String &notes) const = 0;

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

using QueryMatchSet = std::set<AutoQueryMatch, MatchLessCompare>;
using QueryMatchList = Array<AutoQueryMatch>;

//----------------------------------------------------------------------------------------------------------------------

// Text match in an annotation
class Concordance : public QueryMatch
{
public:

	Concordance(const AutoAnnotation &annot, int layer, const AutoEvent &e, const String &text, int pos, String left, String right) :
		QueryMatch(annot, layer, e, text, pos), m_left(std::move(left)), m_right(std::move(right))
	{ }

	String left() const { return m_left; }

	String right() const { return m_right; }

	AutoBookmark to_bookmark(const String &title, const String &notes) const override;

protected:

	// Left and right contexts of the concordance.
	String m_left, m_right;
};

//----------------------------------------------------------------------------------------------------------------------

// Text match according to a coding protocol.
class CodingConcordance final : public Concordance
{
public:

	CodingConcordance(const AutoAnnotation &annot, int layer, const AutoEvent &e, const String &text, int pos,
	                  String left, String right, Array<String> fields) :
		Concordance(annot, layer, e, text, pos, std::move(left), std::move(right)), m_fields(std::move(fields))
	{ }

	String get_field(intptr_t i) const { return m_fields[i]; }

private:

	// Matched text broken down into fields according to the query protocol.
	Array<String> m_fields;
};

//----------------------------------------------------------------------------------------------------------------------

// Base class for all acoustic measurements.
class Measurement : public QueryMatch
{
public:

	Measurement(const AutoAnnotation &annot, int layer, const AutoEvent &e, const String &text, int position) :
			QueryMatch(annot, layer, e, text, position)
	{ }
};

//----------------------------------------------------------------------------------------------------------------------

class FormantMeasurement final : public Measurement
{
public:

	FormantMeasurement(const AutoAnnotation &annot, int layer, const AutoEvent &e, const String &text, int position,
			Array<double> formants) :
			Measurement(annot, layer, e, text, position), m_formants(std::move(formants))
	{ }

private:

	// A matrix where rows represent measurement times and columns represent formants. For example, a measurement with
	// 2 formants measured at 3 time points will be represented by a 3x2 matrix.
	Array<double> m_formants;
};

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_MATCH_HPP
