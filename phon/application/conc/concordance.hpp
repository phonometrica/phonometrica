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
 * Created: 08/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: a Concordance represents the result of a query.                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CONCORDANCE_HPP
#define PHONOMETRICA_CONCORDANCE_HPP

#include <phon/application/dataset.hpp>
#include <phon/application/conc/match.hpp>

namespace phonometrica {

class Concordance : public Dataset
{
public:

	enum class Context
	{
		None,   // no context
		Labels, // labels from surrounding events
		KWIC    // keyword in context
	};

	Concordance(VFolder *parent, const String &path);

	Concordance(intptr_t target_count, Context ctx, intptr_t context_length, Array <AutoMatch> matches, VFolder *parent,
	            const String &path = String());

	const char *class_name() const override;

	bool is_concordance() const override { return true; }

	intptr_t target_count() const { return m_target_count; }

	String get_header(intptr_t j) const override;

	String get_cell(intptr_t i, intptr_t j) const override;

	void set_cell(intptr_t i, intptr_t j, const String &value) override;

	intptr_t row_count() const override;

	intptr_t column_count() const override;

	bool empty() const override;

	void find_context();

	bool has_context() const;

	bool is_match(intptr_t col) const;

	Match &get_match(intptr_t i);

	bool is_file_info_column(intptr_t col) const;

	bool is_metadata_column(intptr_t col) const;

	String label() const override;

	void set_label(String value, bool mutate);

	void modify();

protected:

	void preload();

	void load() override;

	void write() override;

	void parse_options_from_xml(xml_node root);

	void parse_matches_from_xml(xml_node root);

	void find_label_context();

	String get_left_context(intptr_t i) const;

	String get_right_context(intptr_t i) const;

	void find_kwic_context();

	int match_region_size() const;

	int context_column_count() const;

	Array<AutoMatch> m_matches;

	// Left and right context
	Array<std::pair<String,String>> m_context;

	String m_label;

	int m_target_count = 0;

	int m_context_length = 0;

	Context m_context_type = Context::None;
};

using AutoConcordance = std::shared_ptr<Concordance>;

} // namespace phonometrica

#endif // PHONOMETRICA_CONCORDANCE_HPP
