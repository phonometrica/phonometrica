/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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

	double duration() const { return 1000 * (end_time() - start_time()); }

	bool operator<(const QueryMatch &other) const;

	const AutoAnnotation &annotation() const { return m_annot; }

	const AutoEvent &event() const { return m_event; }

	QueryMatch *next() const { return m_next.get(); }

	int layer_index() const { return m_layer_index; }

	int position() const { return m_position; }

	String text() const { return m_text; }

	size_t hash() const;

	virtual AutoBookmark to_bookmark(const String &title, const String &notes) const = 0;

	virtual String left() const { return String(); }

	virtual String right() const { return String(); }

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

	String left() const override { return m_left; }

	String right() const override { return m_right; }

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

	enum Type
	{
		Formants,
		Pitch,
		Intensity
	};

	Measurement(const AutoAnnotation &annot, int layer, const AutoEvent &e, const String &text, int position) :
			QueryMatch(annot, layer, e, text, position)
	{ }

	virtual bool is_pitch() const { return false; }

	virtual bool is_intensity() const { return false; }

	virtual bool is_formants() const { return false; }

	virtual String get_field(intptr_t j) const = 0;

	AutoBookmark to_bookmark(const String &title, const String &notes) const override;

	String get_label(intptr_t i) const;

	void set_labels(Array<String> &&labels) { this->labels = std::move(labels); }

	String left() const override;

	String right() const override;

protected:

	// If [surrounding_labels] is true in the settings, the first two items are the left and right labels that surround the
	// matched item. Other matching labels follow (e.g. syllable, word that are aligned with a vowel), if any.
	Array<String> labels;
};

//----------------------------------------------------------------------------------------------------------------------

class FormantMeasurement final : public Measurement
{
public:

	FormantMeasurement(const AutoAnnotation &annot, int layer, const AutoEvent &e, const String &text, int position,
			double max_freq, int lpc_order, Array<double> formants) :
			Measurement(annot, layer, e, text, position), m_formants(std::move(formants))
	{
		this->max_freq = max_freq;
		this->order = lpc_order;
	}

	bool is_formants() const override { return true; }

	String get_field(intptr_t j) const override;

	intptr_t lpc_order() const { return order; }

	double maximum_frequency() const { return max_freq; }

private:

	// A matrix where rows represent measurement times and columns represent formants. For example, a measurement with
	// 2 formants measured at 3 time points will be represented by a 3x2 matrix.
	Array<double> m_formants;

	// Keep track of these settings, because they vary from measurement to measurement when the automatic method
	// is used.
	double max_freq, order;
};

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_MATCH_HPP
