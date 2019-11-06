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
