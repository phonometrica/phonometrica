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
#include <phon/gui/resampling_dialog.hpp>

namespace phonometrica {

ResamplingDialog::ResamplingDialog(QWidget *parent) : QDialog(parent)
{
	auto layout = new QVBoxLayout;
	layout->addWidget(new QLabel(tr("Choose output file:")));
	file_selector = new FileSelector(tr("Choose output file name"));
	layout->addWidget(file_selector);
	file_selector->setText("/home/julien/test.wav");
	layout->addWidget(new QLabel(tr("New sampling rate (Hz):")));
	fs_line = new QLineEdit;
	fs_line->setText("16000");
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

	connect(button_box, &QDialogButtonBox::accepted, this, &ResamplingDialog::validate);
    connect(button_box, &QDialogButtonBox::rejected, this, &ResamplingDialog::reject);
}

QString ResamplingDialog::format() const
{
	return format_box->currentText();
}

String ResamplingDialog::path() const
{
	QString p = file_selector->text();
	if (! p.isEmpty() && !p.endsWith(format()))
	{
		p.append('.');
		p.append(format());
	}

	return p;
}

void ResamplingDialog::validate()
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
