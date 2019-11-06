/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 12/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: result of a query.                                                                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_TABLE_HPP
#define PHONOMETRICA_QUERY_TABLE_HPP

#include <phon/application/dataset.hpp>
#include <phon/application/search/query.hpp>
#include <phon/application/search/query_match.hpp>
#include <phon/application/protocol.hpp>

namespace phonometrica {

class QueryTable final : public Dataset
{
public:

	enum Flags
	{
		ShowNothing      = 0,
		ShowFields       = 1,
		ShowFileInfo     = 2,
		ShowMatchContext = 4,
		ShowMetadata     = 8,
		ShowAcoustics    = 16
	};

	QueryTable(AutoProtocol p, QueryMatchList matches, String label, const AutoQuerySettings &settings);

	String get_header(intptr_t j) const override;

	String get_cell(intptr_t i, intptr_t j) const override;

	intptr_t row_count() const override;

	intptr_t column_count() const override;

	String label() const override;

	bool empty() const override;

	bool is_query_table() const override { return true; }

	const AutoQueryMatch & get_match(intptr_t i) { return m_matches[i]; }

	void set_flags(int flags) { m_flags = flags; }

	intptr_t adjust_column(intptr_t j) const;

	const char *class_name() const override;

	bool has_textgrid() const;

	bool has_protocol() const { return m_protocol != nullptr; }

	bool has_split_fields() const;

	int field_count() const;

	bool is_acoustic_table() const;

	bool is_text_table() const;

	bool is_formant_table() const;

	bool is_pitch_table() const;

	bool is_intensity_table() const;

private:

	void load() override;

	void write() override;

	intptr_t category_count() const;

	int get_acoustic_field_count() const;

	String get_annotation_name(intptr_t i) const;

	String get_layer_index(intptr_t i) const;

	String get_start_time(intptr_t i) const;

	String get_end_time(intptr_t i) const;

	String get_left_context(intptr_t i) const;

	String get_matched_text(intptr_t i, intptr_t submatch_index = 1) const;

	String get_right_context(intptr_t i) const;

	String get_property(intptr_t i, const String &category) const;

	String get_acoustic_cell(intptr_t i, intptr_t j) const;

	String get_acoustic_header(intptr_t j) const;

	AutoProtocol m_protocol; // may be null

	QueryMatchList m_matches;

	std::set<String> m_categories;

	String m_label;

	AutoQuerySettings m_settings;

	int m_flags = 0;
};

using AutoQueryTable = std::shared_ptr<QueryTable>;

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_TABLE_HPP
