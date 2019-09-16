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
 * Created: 12/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <unordered_set>
#include <phon/application/query_table.hpp>

namespace phonometrica {

static const int BASE_COLUMN_COUNT = 7;
static const int INFO_FILE_COUNT = 4;
static const int CONTEXT_COUNT = 2;

QueryTable::QueryTable(QueryMatchList matches, String label) :
		Dataset(nullptr), m_matches(std::move(matches)), m_categories(Property::get_categories()),
		m_label(std::move(label))
{

}

String QueryTable::get_cell(intptr_t i, intptr_t j) const
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

intptr_t QueryTable::row_count() const
{
	return m_matches.size();
}

intptr_t QueryTable::column_count() const
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

intptr_t QueryTable::category_count() const
{
	return m_categories.size();
}

String QueryTable::get_annotation_name(intptr_t i) const
{
	return m_matches[i]->annotation()->label();
}

String QueryTable::get_layer_index(intptr_t i) const
{
	return String::convert(intptr_t(m_matches[i]->layer_index()));
}

String QueryTable::get_start_time(intptr_t i) const
{
	auto s = String::convert(m_matches[i]->start_time());
	s.chop(8);

	return s;
}

String QueryTable::get_end_time(intptr_t i) const
{
	auto s = String::convert(m_matches[i]->end_time());
	s.chop(8);

	return s;
}

String QueryTable::get_left_context(intptr_t i) const
{
	return dynamic_cast<Concordance*>(m_matches[i].get())->left();
}

String QueryTable::get_matched_text(intptr_t i, intptr_t submatch_index) const
{
	return m_matches[i]->text();
}

String QueryTable::get_right_context(intptr_t i) const
{
	return dynamic_cast<Concordance*>(m_matches[i].get())->right();
}

String QueryTable::get_property(intptr_t i, const String &category) const
{
	return m_matches[i]->annotation()->get_property_value(category);
}

String QueryTable::label() const
{
	return m_label;
}

String QueryTable::get_header(intptr_t j) const
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

bool QueryTable::empty() const
{
	return m_matches.empty();
}

intptr_t QueryTable::adjust_column(intptr_t j) const
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

void QueryTable::load()
{

}

void QueryTable::write()
{
	// TODO: read/write query table from/to native format
#if 0
	assert(!m_path.empty());
	int flags = m_flags;
	m_flags = 0;

	xml_document doc;
	auto root = doc.append_child("Phonometrica");
	auto attr = root.append_attribute("class");
    attr.set_value(class_name());
    auto data_node = root.append_child("Data");
    attr = data_node.append_attribute("size");
	attr.set_value(m_matches.size());
	auto nrow = this->row_count();
	auto ncol = BASE_COLUMN_COUNT;

	Array<String> headers;
	headers.append("Annotation");
	headers.append("Layer");
	headers.append("StartTime");
	headers.append("EndTime");
	headers.append("LeftContext");
	headers.append("Match");
	headers.append("RightContext");

	for  (intptr_t j = 1; j <= ncol; j++) headers.append(get_header(j));

	for (intptr_t i = 1; i <= nrow; i++)
	{
		auto node = data_node.append_child("QueryMatch");

		for (intptr_t j = 1; j <= ncol; j++)
		{
			auto subnode = node.append_child(headers[j].data());
			auto data = subnode.append_child(node_pcdata);
			auto field = get_cell(i, j);
			data.set_value(field.data());
		}
	}

    write_xml(doc, m_path);
    m_flags = flags;
#endif
}

const char *QueryTable::class_name() const
{
	return "QueryTable";
}

} // namespace phonometrica
