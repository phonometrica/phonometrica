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
 * Created: 12/09/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <unordered_set>
#include <phon/application/query_dataset.hpp>

namespace phonometrica {

static const int BASE_COLUMN_COUNT = 7;
static const int INFO_FILE_COUNT = 4;
static const int CONTEXT_COUNT = 2;

QueryDataset::QueryDataset(QueryMatchList matches, String label) :
		Dataset(nullptr), m_matches(std::move(matches)), m_categories(Property::get_categories()),
		m_label(std::move(label))
{

}

String QueryDataset::get_cell(intptr_t i, intptr_t j) const
{
	j = adjust_column(j);

	switch (j)
	{
		case 1:
			return get_annotation_name(i);
		case 2:
			return get_layer_index(i);
		case 3:
			return get_start_time(i);
		case 4:
			return get_end_time(i);
		case 5:
			return get_left_context(i);
		case 6:
			return get_matched_text(i);
		case 7:
			return get_right_context(i);
		default:
			break;
	}
	auto it = m_categories.begin();
	std::advance(it, (j - BASE_COLUMN_COUNT - 1));
	auto &category = *it;
//	auto view = category.view();
	return get_property(i, category);
}

intptr_t QueryDataset::row_count() const
{
	return m_matches.size();
}

intptr_t QueryDataset::column_count() const
{
	/*
	 * A match has the following columns for a simple query:
	 * - file (ShowFileInfo)
	 * - layer (ShowFileInfo)
	 * - start time (ShowFileInfo)
	 * - end time (ShowFileInfo)
	 * - left context (ShowMatchContext)
	 * - match
	 * - right context (ShowMatchContext)
	 * - one additional column per property category (ShowProperties)
	 */
	intptr_t col_count = 1;

	if (m_flags & ShowFileInfo)
		col_count += INFO_FILE_COUNT;
	if (m_flags & ShowMatchContext)
		col_count += CONTEXT_COUNT;
	if (m_flags & ShowProperties)
		col_count += category_count();

	return col_count;
}

intptr_t QueryDataset::category_count() const
{
	return m_categories.size();
}

String QueryDataset::get_annotation_name(intptr_t i) const
{
	return m_matches[i]->annotation()->label();
}

String QueryDataset::get_layer_index(intptr_t i) const
{
	return String::convert(intptr_t(m_matches[i]->layer_index()));
}

String QueryDataset::get_start_time(intptr_t i) const
{
	auto s = String::convert(m_matches[i]->start_time());
	s.chop(8);

	return s;
}

String QueryDataset::get_end_time(intptr_t i) const
{
	auto s = String::convert(m_matches[i]->end_time());
	s.chop(8);

	return s;
}

String QueryDataset::get_left_context(intptr_t i) const
{
	return dynamic_cast<Concordance*>(m_matches[i].get())->left();
}

String QueryDataset::get_matched_text(intptr_t i, intptr_t submatch_index) const
{
	return m_matches[i]->text();
}

String QueryDataset::get_right_context(intptr_t i) const
{
	return dynamic_cast<Concordance*>(m_matches[i].get())->right();
}

String QueryDataset::get_property(intptr_t i, const String &category) const
{
	return m_matches[i]->annotation()->get_property_value(category);
}

String QueryDataset::label() const
{
	return m_label;
}

String QueryDataset::get_header(intptr_t j) const
{
	j = adjust_column(j);

	switch (j)
	{
		case 1:
			return "File";
		case 2:
			return "Layer";
		case 3:
			return "Start time";
		case 4:
			return "End time";
		case 5:
			return "Left context";
		case 6:
			return "Match";
		case 7:
			return "Right context";
		default:
			break;
	}
	auto it = m_categories.begin();
	std::advance(it, (j - BASE_COLUMN_COUNT - 1));
	auto &category = *it;

	return category;
}

bool QueryDataset::empty() const
{
	return m_matches.empty();
}

intptr_t QueryDataset::adjust_column(intptr_t j) const
{
	if (!(m_flags & ShowFileInfo))
		j += INFO_FILE_COUNT;
	if (!(m_flags & ShowMatchContext))
	{
		if (j == 5) // match
			j += 1; // account for left context
		else if (j > 5)
			j += 2; // account for left and right contexts
	}

	return j;
}

} // namespace phonometrica
