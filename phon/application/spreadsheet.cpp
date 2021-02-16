/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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

Spreadsheet::Spreadsheet(Directory *parent, String path) :
		Dataset(get_class_ptr<Spreadsheet>(), parent, std::move(path))
{

}

Spreadsheet::Spreadsheet(const Spreadsheet &other) :
	Dataset(other.klass, other.parent(), String()), m_labels(other.m_labels)
{
	for (auto &col : other.m_columns) {
		m_columns.append(std::unique_ptr<Column>(col->clone()));
	}

	nrow = other.nrow;
	ncol = other.ncol;

	m_content_modified = true;
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

void Spreadsheet::set_cell(intptr_t i, intptr_t j, const String &value)
{
	auto col = m_columns[j].get();

	switch (col->type())
	{
		case Type::Numeric:
		{
			bool ok;
			double result = value.to_float(&ok);
			if (!ok) {
				throw error("Invalid numeric value in cell (%, %)", i, j);
			}
			cast_num(col)->set(i, result);
		}
		break;
		case Type::Boolean:
		{
			cast_bool(col)->set(i, value.to_bool());
		}
		break;
		default:
			cast_string(col)->set(i, value);
	}
}

const char *Spreadsheet::class_name() const
{
	// TODO: rename to Spreadsheet once refactoring is done
	return "Dataset";
}

void Spreadsheet::initialize(Runtime &)
{

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