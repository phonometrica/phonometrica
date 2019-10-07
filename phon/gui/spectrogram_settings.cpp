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
 * Created: 07/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QMessageBox>
#include <QDialogButtonBox>
#include <QLayout>
#include <QLabel>
#include <phon/gui/spectrogram_settings.hpp>

namespace phonometrica {

SpectrogramSettings::SpectrogramSettings(Runtime &rt, QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Spectrogram settings..."));
	setMinimumWidth(300);

	auto layout = new QVBoxLayout;
	layout->addWidget(new QLabel(tr("Spectrogram type:")));
	wide_button = new QRadioButton(tr("Wide-band (5 ms)"));
	wide_button->setChecked(true);
	narrow_button = new QRadioButton(tr("Narrow-band (25 ms)"));
	auto custom_layout = new QHBoxLayout;
	custom_button = new QRadioButton(tr("Custom window size (in ms):"));
	custom_edit = new QLineEdit;
	auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

	contrast_slider = new QSlider(Qt::Horizontal);
	contrast_slider->setMinimum(0);
	contrast_slider->setMaximum(255);
	contrast_slider->setSingleStep(1);
	contrast_slider->setValue(70);

	layout->addWidget(wide_button);
	layout->addWidget(narrow_button);
	custom_layout->addWidget(custom_button);
	custom_layout->addWidget(custom_edit);
	layout->addLayout(custom_layout);
	layout->addWidget(new QLabel("Dynamic range (dB):"));
	layout->addWidget(contrast_slider);
	layout->addWidget(button_box);
	setLayout(layout);

	connect(button_box, &QDialogButtonBox::accepted, this, &SpectrogramSettings::accept);
    connect(button_box, &QDialogButtonBox::rejected, this, &SpectrogramSettings::reject);

}

double SpectrogramSettings::windowSize() const
{
	if (wide_button->isChecked())
		return 0.005;
	if (narrow_button->isChecked())
		return 0.025;
	bool ok;
	int value = custom_edit->text().toInt(&ok);

	if (!ok || value <= 0) {
		QMessageBox dlg(QMessageBox::Critical, tr("Invalid setting"), tr("Invalid window size"));
		dlg.exec();
		return 0.005;
	}

	return double(value) / 1000;
}
} // namespace phonometrica
