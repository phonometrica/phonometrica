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

#include <phon/application/conc/query.hpp>

namespace phonometrica {

Query::Query(VFolder *parent, String path) :
	VFile(parent, std::move(path))
{

}

void Query::add_metaconstraint(AutoMetaConstraint m, bool mutate)
{
	m_metaconstraints.append(std::move(m));
	if (mutate) m_content_modified = true;
}

void Query::add_constraint(Constraint c, bool mutate)
{
	m_constraints.append(std::move(c));
	if (mutate) m_content_modified = true;
}

Array<AutoAnnotation> Query::filter_annotations(const Array<AutoAnnotation> &candidates) const
{
	if (m_metaconstraints.empty()) {
		return candidates;
	}
	Array<AutoAnnotation> result;

	for (auto &candidate : candidates)
	{
		if (filter_metadata(candidate.get())) {
			result.append(candidate);
		}
	}

	return result;
}

bool Query::filter_metadata(const VFile *file) const
{
	// Reject files that do not satisfy all the constraints.
	for (auto &constraint : m_metaconstraints)
	{
		if (!constraint->filter(file)) {
			return false;
		}
	}

	return true;
}

String Query::label() const
{
	return m_label;
}

void Query::set_label(String value, bool mutate)
{
	m_label = std::move(value);
	if (mutate) m_content_modified = true;
}

bool Query::is_query() const
{
	return true;
}

void Query::set_selection(Array<AutoAnnotation> files)
{
	selected_annotations = std::move(files);
	m_content_modified = true;
}

void Query::clear()
{
	m_metaconstraints.clear();
	m_constraints.clear();
	selected_annotations.clear();
	m_label = String();
	m_content_modified = true;
}

} // namespace phonometrica
