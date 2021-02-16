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

class Query : public Document
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

	using Context = Concordance::Context;

	Query(Directory *parent, String path);

	~Query() override = default;

	void add_metaconstraint(AutoMetaConstraint m, bool mutate = true);

	void add_constraint(Constraint c, bool mutate = true);

	String label() const override;

	void set_label(String value, bool mutate);

	bool is_query() const override;

	void set_selection(Array<Handle<Annotation>> files);

	virtual void clear();

	virtual Handle<Concordance> execute();

	// Note: subclasses must override this method and return false
	virtual bool is_text_query() const { return true; }

	virtual bool is_duration_query() const { return false; }

	virtual bool is_formant_query() const { return false; }

	virtual bool is_pitch_query() const { return false; }

	virtual bool is_intensity_query() const { return false; }

	const Array<AutoMetaConstraint> &metaconstraints() const { return m_metaconstraints; }

	const Array<Handle<Annotation>> &selection() const { return selected_annotations; }

	intptr_t constraint_count() const { return m_constraints.size(); }

	const Constraint &get_constraint(intptr_t i) const { return m_constraints[i]; }

	virtual Handle<Query> copy() const;

	int context_length() const;

	void set_context_length(int context_length);

	Context context() const;

	void set_context(Context context);

	int reference_constraint() const;

	void set_reference_constraint(int value);

	bool empty();

	static void initialize(Runtime &rt);

protected:

	void load() override;

	void write() override;

	void parse_metaconstraints_from_xml(xml_node root);

	void parse_constraints_from_xml(xml_node root);

	void parse_options_from_xml(xml_node root);

	Array<Handle<Annotation>> filter_annotations(Array<Handle<Annotation>> candidates) const;

	bool filter_metadata(const Document *file) const;

	Array<AutoMatch> search();

	Array<AutoMatch> search_annotation(const Handle<Annotation> &annot);

	Array <AutoMatch> find_matches(const Handle<Annotation> &annot, const Constraint &constraint, Array <AutoMatch> matches,
	                               Array<int> &blacklist, Constraint::Operator op, bool is_ref) const;

	Array <AutoMatch> find_matches(const Handle<Annotation> &annot, const Constraint &constraint, Array <AutoMatch> matches,
	                               intptr_t layer_index,
	                               Array<int> &seen, Constraint::Operator op, bool is_ref) const;

	std::unique_ptr<Match::Target>
	find_target(const AutoEvent &event, const Constraint &constraint, intptr_t layer_index, intptr_t &pos,
	            bool is_ref) const;

	// Constraints on the metadata
	Array<AutoMetaConstraint> m_metaconstraints;

	// Constraints on the data
	Array<Constraint> m_constraints;

	// If empty, use all the annotations from the project
	Array<Handle<Annotation>> selected_annotations;

	// Label set by the user
	String m_label;

	// Type of context for the reference constraint
	Context m_context = Context::None;

	// Reference constraint
	int m_ref_constraint;

	// Context length, for KWIC mode
	int m_context_length = 0;
};


namespace traits {
template<> struct maybe_cyclic<Query> : std::false_type { };
}

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_HPP
