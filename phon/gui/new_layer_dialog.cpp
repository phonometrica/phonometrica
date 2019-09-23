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