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
 * Created: 03/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <phon/gui/csv_import_dialog.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

CsvImportDialog::CsvImportDialog(QWidget *parent, Runtime &rt) :
	QDialog(parent), runtime(rt)
{
	setWindowTitle(tr("Choose CSV file..."));
	setMinimumWidth(350);
	auto hlayout = new QHBoxLayout;
	path_edit = new QLineEdit;
	auto path_button = new QPushButton(tr("Choose..."));
	hlayout->addWidget(path_edit);
	hlayout->addWidget(path_button);

	semicolon_button = new QRadioButton(tr("semi-colon"));
	comma_button = new QRadioButton(tr("comma"));
	tab_button = new QRadioButton(tr("tabulation"));
	semicolon_button->setChecked(true);

	auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

	auto layout = new QVBoxLayout;
	layout->addWidget(new QLabel(tr("Path:")));
	layout->addLayout(hlayout);
	layout->addSpacing(10);
	layout->addWidget(new QLabel(tr("Separator:")));
	layout->addWidget(semicolon_button);
	layout->addWidget(comma_button);
	layout->addWidget(tab_button);
	layout->addWidget(button_box);
	setLayout(layout);

	connect(path_button, &QPushButton::clicked, this, &CsvImportDialog::setPath);
	connect(button_box, &QDialogButtonBox::accepted, this, &CsvImportDialog::accept);
	connect(button_box, &QDialogButtonBox::rejected, this, &CsvImportDialog::reject);
}

void CsvImportDialog::setPath(bool)
{
	auto dir = Settings::get_string(runtime, "last_directory");
	QString path = QFileDialog::getOpenFileName(this->parentWidget(), tr("Choose file..."), dir, "CSV (*.csv);; TXT (*.txt)");

	if (!path.isEmpty())
		path_edit->setText(path);
}

String CsvImportDialog::separator() const
{
	if (semicolon_button->isChecked())
		return ";";
	if (comma_button->isChecked())
		return ",";

	return "\t";
}

String CsvImportDialog::path() const
{
	return path_edit->text();
}
} // namespace phonometrica
