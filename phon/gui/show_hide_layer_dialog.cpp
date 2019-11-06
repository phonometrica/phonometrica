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
 * Created: 22/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLayout>
#include <QDialogButtonBox>
#include <phon/gui/show_hide_layer_dialog.hpp>

namespace phonometrica {

ShowHideLayerDialog::ShowHideLayerDialog(const Array<String> &labels, const Array<bool> &visibility, QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle("Select visible layers");
	auto layout = new QVBoxLayout;
	list_box = new CheckListBox("Layers", labels);
	layout->addWidget(list_box);
	bool all_visible = true;
	for (auto v : visibility)
	{
		if (!v)
		{
			all_visible = false;
			break;
		}
	}
	if (all_visible)
	{
		list_box->setChecked(true);
	}
	else
	{
		for (int i = 1; i <= visibility.size(); i++)
		{
			list_box->checkItem(i-1, visibility[i]);
		}
	}

	auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	layout->addWidget(button_box);
	setLayout(layout);

	connect(button_box, &QDialogButtonBox::accepted, this, &ShowHideLayerDialog::accept);
	connect(button_box, &QDialogButtonBox::rejected, this, &ShowHideLayerDialog::reject);
}

Array<int> ShowHideLayerDialog::selectedIndexes()
{
	return list_box->checkedIndexes();
}
} // namespace phonometrica
