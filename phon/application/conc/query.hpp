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
 * Purpose: Base class for a all queries. A Query is a blueprint for the search engine. Each execution of a given      *
 * query produces a Concordance.                                                                                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_HPP
#define PHONOMETRICA_QUERY_HPP

#include <phon/application/annotation.hpp>
#include <phon/application/conc/metaconstraint.hpp>
#include <phon/application/conc/constraint.hpp>

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

	Query(VFolder *parent, String path);

	~Query() override = default;

	void add_metaconstraint(std::unique_ptr<MetaConstraint> m, bool mutate = true);

	void add_constraint(Constraint c, bool mutate = true);

	String label() const override;

	void set_label(String value, bool mutate);

	bool is_query() const override;

	void set_selection(Array<AutoAnnotation> files);

	virtual void clear();

	virtual bool is_text_query() const { return false; }

	virtual bool is_duration_query() const { return false; }

	virtual bool is_formant_query() const { return false; }

	virtual bool is_pitch_query() const { return false; }

	virtual bool is_intensity_query() const { return false; }

	const Array<std::unique_ptr<MetaConstraint>> &metaconstraints() const { return m_metaconstraints; }

	const Array<AutoAnnotation> &selection() const { return selected_annotations; }

	intptr_t constraint_count() const { return m_constraints.size(); }

	const Constraint &get_constraint(intptr_t i) const { return m_constraints[i]; }

protected:

	Array<AutoAnnotation> filter_annotations(const Array<AutoAnnotation> &candidates) const;

	bool filter_metadata(const VFile *file) const;

	// Constraints on the metadata.
	Array<std::unique_ptr<MetaConstraint>> m_metaconstraints;

	// Constraints on the data
	Array<Constraint> m_constraints;

	// If empty, use all the annotations from the project
	Array<AutoAnnotation> selected_annotations;

	// Label set by the user
	String m_label;
};


using AutoQuery = std::shared_ptr<Query>;

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_HPP
