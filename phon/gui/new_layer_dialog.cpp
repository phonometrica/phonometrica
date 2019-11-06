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
 * Created: 23/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLabel>
#include <QLayout>
#include <QDialogButtonBox>
#include <phon/gui/new_layer_dialog.hpp>

namespace phonometrica {

NewLayerDialog::NewLayerDialog(QWidget *parent, int layer_count) :
	QDialog(parent)
{
	setWindowTitle(tr("Add new layer..."));
	auto layout = new QGridLayout;

	name_edit = new QLineEdit;

	type_combo = new QComboBox;
	type_combo->addItem(tr("intervals"));
	type_combo->addItem(tr("instants"));
	type_combo->setCurrentIndex(0);

	index_box = new QSpinBox;
	index_box->setMinimum(1);
	index_box->setMaximum(layer_count+1);
	index_box->setValue(layer_count+1);

	auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	connect(button_box, &QDialogButtonBox::accepted, this, &NewLayerDialog::accept);
	connect(button_box, &QDialogButtonBox::rejected, this, &NewLayerDialog::reject);

	layout->addWidget(new QLabel(tr("Name:")), 0, 0);
	layout->addWidget(name_edit, 0, 1);
	layout->addWidget(new QLabel(tr("Event type:")), 1, 0);
	layout->addWidget(type_combo, 1, 1);
	layout->addWidget(new QLabel(tr("Layer index:")), 2, 0);
	layout->addWidget(index_box, 2, 1);

	auto main_layout = new QVBoxLayout;
	main_layout->addLayout(layout);
	main_layout->addWidget(button_box);
	setLayout(main_layout);
}

} // namespace phonometrica