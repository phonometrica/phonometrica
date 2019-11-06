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
 * Created: 21/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <phon/application/sound.hpp>
#include <phon/gui/conversion_dialog.hpp>

namespace phonometrica {

ConversionDialog::ConversionDialog(int sample_rate, QWidget *parent) : QDialog(parent)
{
	auto layout = new QVBoxLayout;
	layout->addWidget(new QLabel(tr("Choose output file:")));
	file_selector = new FileSelector(tr("Choose output file name"));
	layout->addWidget(file_selector);
	layout->addWidget(new QLabel(tr("New sampling rate (Hz):")));
	fs_line = new QLineEdit;
	fs_line->setText(QString::number(sample_rate));
	layout->addWidget(fs_line);
	format_box = new QComboBox;
	format_box->addItem("wav");
	format_box->addItem("aiff");
	if (Sound::common_sound_formats().contains("flac")) {
		format_box->addItem("flac");
	}
	format_box->setCurrentIndex(0);
	layout->addWidget(format_box);
	auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	layout->addWidget(button_box);
	setLayout(layout);

	connect(button_box, &QDialogButtonBox::accepted, this, &ConversionDialog::validate);
    connect(button_box, &QDialogButtonBox::rejected, this, &ConversionDialog::reject);
}

QString ConversionDialog::format() const
{
	return format_box->currentText();
}

String ConversionDialog::path() const
{
	QString p = file_selector->text();
	if (! p.isEmpty() && !p.endsWith(format()))
	{
		p.append('.');
		p.append(format());
	}

	return p;
}

void ConversionDialog::validate()
{
	if (file_selector->text().isEmpty()) {
		QMessageBox::critical(this, tr("Invalid resampling settings"), tr("Empty output path"));
		return;
	}
	bool ok;
	Fs = fs_line->text().toInt(&ok);
	if (!ok || Fs <= 0) {
		QMessageBox::critical(this, tr("Invalid resampling settings"), tr("Invalid sampling rate"));
		return;
	}
	accept();
}

} // namespace phonometrica
