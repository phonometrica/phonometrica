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
 * Created: 17/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: display a spreadsheet.                                                                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SPREADSHEET_VIEW_HPP
#define PHONOMETRICA_SPREADSHEET_VIEW_HPP

#include <QTableWidget>
#include <phon/application/spreadsheet.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/gui/views/view.hpp>

namespace phonometrica {

class SpreadsheetView final : public View
{
	Q_OBJECT

public:

	SpreadsheetView(QWidget *parent, Runtime &rt, AutoSpreadsheet data);

	bool save() override;

private:

	void setupUi();

	Runtime &runtime;

	QTableWidget *m_table;

	AutoSpreadsheet m_data;
};

} // namespace phonometrica

#endif // PHONOMETRICA_SPREADSHEET_VIEW_HPP
