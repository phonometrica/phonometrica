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

#include <phon/application/spreadsheet.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/utils/text.hpp>

namespace phonometrica {

Spreadsheet::Column::~Column()
{

}

void Spreadsheet::load()
{
	auto ext = filesystem::ext(m_path, true);

	if (ext == ".csv")
	{
		read_from_csv("\t");
	}
	else
	{
		throw error("Cannot load spreadsheet with '%' extension", ext);
	}
}

void Spreadsheet::write()
{

}

void Spreadsheet::read_from_csv(std::string_view sep)
{
	assert(!m_path.empty());
	auto raw_data = utils::parse_csv(m_path, sep);
	if (raw_data.empty()) return;

	// Parse header.
	for (auto &label : raw_data.take_first())
	{
		if (label.ends_with(".num"))
		{
			label.remove_last(".num");
			m_columns.append(std::make_unique<TColumn<double>>());
		}
		else if (label.ends_with(".bool"))
		{
			label.remove_last(".bool");
			m_columns.append(std::make_unique<TColumn<bool>>());
		}
		else
		{
			if (label.ends_with(".text")) label.remove_last(".text");
			m_columns.append(std::make_unique<TColumn<String>>());
		}
		m_labels.append(std::move(label));
	}

	// Parse data.
	if (raw_data.empty()) return;
	nrow = raw_data.size();
	ncol = m_labels.size();
	for (auto &col : m_columns) col->resize(nrow);

	for (intptr_t i = 1; i <= nrow; i++)
	{
		auto &row = raw_data[i];

		for (intptr_t j = 1; j <= ncol; j++)
		{
			auto col = m_columns[j].get();

			switch (m_columns[j]->type())
			{
				case Type::Numeric:
				{
					double value = row[j].to_float();
					cast_num(col)->set(i, value);
					break;
				}
				case Type::Boolean:
				{
					bool value = row[j].to_bool();
					cast_bool(col)->set(i, value);
					break;
				}
				default:
				{
					cast_string(col)->set(i, std::move(row[j]));
				}
			}
		}
	}
}

String Spreadsheet::get_header(intptr_t j) const
{
	return m_labels[j];
}

String Spreadsheet::get_cell(intptr_t i, intptr_t j) const
{
	auto col = m_columns[j].get();

	switch (col->type())
	{
		case Type::Numeric:
			return String::convert(cast_num(col)->get(i));
		case Type::Boolean:
			return String::convert(cast_bool(col)->get(i));
		default:
			return cast_string(col)->get(i);
	}
}

Spreadsheet::Type Spreadsheet::Column::find_type(const std::type_info &t) const
{
	if (t == typeid(String))
		return Type::Text;
	if (t == typeid(double))
		return Type::Numeric;
	assert(t == typeid(bool));
	return Type::Boolean;
}

} // namespace phonometrica