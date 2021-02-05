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
 * Purpose: Query for text concordances in annotations.                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TEXT_QUERY_HPP
#define PHONOMETRICA_TEXT_QUERY_HPP

#include <phon/application/conc/query.hpp>

namespace phonometrica {

class TextQuery final : public Query
{
public:

	enum class Context
	{
		None,   // no context
		Labels, // labels from surrounding events
		KWIC    // keyword in context
	};

	TextQuery(VFolder *parent, const String &path);

	const char *class_name() const override { return "TextQuery"; }

	// Note: subclasses must override this method and return false
	bool is_text_query() const override { return true; }

	int context_length() const;

	void set_context_length(int context_length);

	Context context() const;

	void set_context(Context context);

	int reference_constraint() const;

	void set_reference_constraint(int value);

	AutoQuery clone() const override;

private:

	void load() override;

	void write() override;

	void parse_metaconstraints_from_xml(xml_node root);

	void parse_constraints_from_xml(xml_node root);

	void parse_options_from_xml(xml_node root);

	Context m_context = Context::KWIC;

	int m_ref_constraint;

	int m_context_length = 0;
};

using AutoTextQuery = std::shared_ptr<TextQuery>;

} // namespace phonometrica

#endif // PHONOMETRICA_TEXT_QUERY_HPP
