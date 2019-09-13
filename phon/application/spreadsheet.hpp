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
 * Purpose: tabular dataset (e.g. CSV file).                                                                          *
 *                                                                                                                    *
 **********************************************************************************************************************/

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

	String get_header(intptr_t j) const override { return String(); }

	String get_cell(intptr_t i, intptr_t j) const override { return String(); }

	intptr_t row_count() const override { return 0; }

	intptr_t column_count() const override { return 0; }

	bool empty() const override { return true; } // TODO: Spreadsheet::empty();

	bool is_query_dataset() const override { return false; }

	bool is_spreadsheet() const override { return true; }

private:

};

} // namespace phonometrica

#endif // PHONOMETRICA_SPREADSHEET_HPP
