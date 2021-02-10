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
 * Purpose: tabular dataset (e.g. CSV file).                                                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SPREADSHEET_HPP
#define PHONOMETRICA_SPREADSHEET_HPP

#include <phon/application/dataset.hpp>

namespace phonometrica {

class Spreadsheet final : public Dataset
{
public:

	Spreadsheet(VFolder *parent, String path = String()) :
		Dataset(parent, std::move(path))
	{ }

	String get_header(intptr_t j) const override;

	String get_cell(intptr_t i, intptr_t j) const override;

	void set_cell(intptr_t i, intptr_t j, const String &value) override;

	const char *class_name() const override;

	intptr_t row_count() const override { return nrow; }

	intptr_t column_count() const override { return ncol; }

	bool empty() const override { return nrow == 0; }

	bool is_spreadsheet() const override { return true; }

private:

	enum class Type
	{
		Boolean,
		Numeric,
		Text
	};

	struct Column
	{
		virtual ~Column();

		virtual Type type() const = 0;

		virtual void resize(intptr_t size) = 0;

	protected:

		Type find_type(const std::type_info &t) const;
	};

	template<class T>
	struct TColumn : public Column
	{
		TColumn() = default;

		TColumn(intptr_t size, const T &value = T()) :
			data(size, value)
		{ }

		Type type() const override { return find_type(typeid(T)); }

		const T &get(intptr_t i) const { return data[i]; }

		void set(intptr_t i, T value) { data[i] = std::move(value); }

		void resize(intptr_t size) override { data.resize(size); }

		Array<T> data;
	};

	void read_from_csv(std::string_view sep = ",");

	void load() override;

	void write() override;

	TColumn<double>* cast_num(Column *col) { return static_cast<TColumn<double>*>(col); }
	const TColumn<double>* cast_num(Column *col) const { return static_cast<TColumn<double>*>(col); }

	TColumn<bool>* cast_bool(Column *col) { return static_cast<TColumn<bool>*>(col); }
	const TColumn<bool>* cast_bool(Column *col) const { return static_cast<TColumn<bool>*>(col); }

	TColumn<String>* cast_string(Column *col) { return static_cast<TColumn<String>*>(col); }
	const TColumn<String>* cast_string(Column *col) const { return static_cast<TColumn<String>*>(col); }

	using AutoColumn = std::unique_ptr<Column>;

	Array<String> m_labels;

	Array<AutoColumn> m_columns;

	intptr_t nrow = 0;

	intptr_t ncol = 0;
};

using AutoSpreadsheet = std::shared_ptr<Spreadsheet>;

namespace traits {
template<> struct maybe_cyclic<AutoSpreadsheet> : std::false_type { };
}

} // namespace phonometrica

#endif // PHONOMETRICA_SPREADSHEET_HPP
