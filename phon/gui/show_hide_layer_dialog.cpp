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
	setWindowTitle("Show/hide layers");
	auto layout = new QVBoxLayout;
	list_box = new CheckListBox("Select visible layers", labels);
	layout->addWidget(list_box);
	for (int i = 1; i <= visibility.size(); i++)
	{
		list_box->checkItem(i-1, visibility[i]);
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
