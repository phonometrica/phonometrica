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
 * Purpose: Base class for a all queries. By default, searches for matches in a set of annotations. Subclasses can     *
 * additionally take phonetic measurements.                                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_HPP
#define PHONOMETRICA_QUERY_HPP

#include <phon/application/annotation.hpp>
#include <phon/application/conc/metaconstraint.hpp>
#include <phon/application/conc/constraint.hpp>
#include <phon/application/conc/concordance.hpp>
#include <phon/regex.hpp>

namespace phonometrica {

class Query : public VFile
{
public:

	enum class Type
	{
		Text      = 1,
		Formant   = 2,
		Pitch     = 4,
		Intensity = 8,
		Duration  = 16,
		Acoustic  = Formant|Pitch|Intensity|Duration
	};

	enum class Context
	{
		None,   // no context
		Labels, // labels from surrounding events
		KWIC    // keyword in context
	};

	Query(VFolder *parent, String path);

	~Query() override = default;

	const char *class_name() const override { return "Query"; }

	void add_metaconstraint(AutoMetaConstraint m, bool mutate = true);

	void add_constraint(Constraint c, bool mutate = true);

	String label() const override;

	void set_label(String value, bool mutate);

	bool is_query() const override;

	void set_selection(Array<AutoAnnotation> files);

	virtual void clear();

	virtual AutoConcordance execute();

	// Note: subclasses must override this method and return false
	virtual bool is_text_query() const { return true; }

	virtual bool is_duration_query() const { return false; }

	virtual bool is_formant_query() const { return false; }

	virtual bool is_pitch_query() const { return false; }

	virtual bool is_intensity_query() const { return false; }

	const Array<AutoMetaConstraint> &metaconstraints() const { return m_metaconstraints; }

	const Array<AutoAnnotation> &selection() const { return selected_annotations; }

	intptr_t constraint_count() const { return m_constraints.size(); }

	const Constraint &get_constraint(intptr_t i) const { return m_constraints[i]; }

	virtual std::shared_ptr<Query> clone() const;

	int context_length() const;

	void set_context_length(int context_length);

	Context context() const;

	void set_context(Context context);

	int reference_constraint() const;

	void set_reference_constraint(int value);

protected:

	void load() override;

	void write() override;

	void parse_metaconstraints_from_xml(xml_node root);

	void parse_constraints_from_xml(xml_node root);

	void parse_options_from_xml(xml_node root);

	Array<AutoAnnotation> filter_annotations(const Array<AutoAnnotation> &candidates) const;

	bool filter_metadata(const VFile *file) const;

	Array<AutoMatch> search(const Annotation &annot);

	Array<AutoMatch> search(const Annotation &annot, const Constraint &constraint, Regex *layer_pattern) const;

	Array<AutoMatch> find_matches(const Annotation &annot, const Constraint &constraint, Array<AutoMatch> matches) const;

	Array<AutoMatch> find_matches(const Annotation &annot, const Constraint &constraint, Array<AutoMatch> matches, intptr_t layer) const;

	// Constraints on the metadata
	Array<AutoMetaConstraint> m_metaconstraints;

	// Constraints on the data
	Array<Constraint> m_constraints;

	// If empty, use all the annotations from the project
	Array<AutoAnnotation> selected_annotations;

	// Label set by the user
	String m_label;

	// Type of context for the reference constraint
	Context m_context = Context::KWIC;

	// Reference constraint
	int m_ref_constraint;

	// Context length, for KWIC mode
	int m_context_length = 0;
};


using AutoQuery = std::shared_ptr<Query>;

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_HPP
