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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLayout>
#include <phon/gui/views/spreadsheet_view.hpp>

namespace phonometrica {

SpreadsheetView::SpreadsheetView(QWidget *parent, Runtime &rt, AutoSpreadsheet data) :
	View(parent), runtime(rt), m_data(std::move(data))
{
	m_data->open();
	setupUi();
}

bool SpreadsheetView::save()
{
	return true;
	//m_data->save();
}

void SpreadsheetView::setupUi()
{
	auto layout = new QVBoxLayout;
	int nrow = m_data->row_count();
	int ncol = m_data->column_count();
	m_table = new QTableWidget(nrow, ncol, this);
	QFont font("Noto Sans Mono");
	//font.setStyleHint(QFont::Monospace);
	QStringList vheaders, hheaders;
	vheaders.reserve(m_table->rowCount());
	hheaders.reserve(m_table->columnCount());

	for (int j = 1; j <= m_table->columnCount(); j++)
	{
		hheaders << m_data->get_header(j);
	}

	for (int i = 1; i <= m_table->rowCount(); i++)
	{
		vheaders << QString("%1").arg(i);

		for (int j = 1; j <= m_table->columnCount(); j++)
		{
			auto label = m_data->get_cell(i, j);
			auto item = new QTableWidgetItem(label);
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			item->setFont(font);
			m_table->setItem(i-1, j-1, item);
		}
	}

	m_table->setHorizontalHeaderLabels(hheaders);
	m_table->setVerticalHeaderLabels(vheaders);
	m_table->resizeColumnsToContents();

	layout->addWidget(m_table);
	setLayout(layout);
}
} // namespace phonometrica
