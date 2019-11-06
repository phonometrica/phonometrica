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
 * Created: 07/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QMessageBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLayout>
#include <phon/runtime/runtime.hpp>
#include <phon/application/settings.hpp>
#include <phon/include/reset_spectrogram_settings_phon.hpp>
#include <phon/gui/spectrogram_settings.hpp>

namespace phonometrica {

SpectrogramSettings::SpectrogramSettings(Runtime &rt, QWidget *parent) :
	QDialog(parent), runtime(rt)
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

	range_edit = new QLineEdit;

	window_box = new QComboBox;
	window_box->addItem("Bartlett");
    window_box->addItem("Blackman");
    window_box->addItem("Gaussian");
	window_box->addItem("Hamming");
	window_box->addItem("Hann");
	window_box->addItem("Rectangular");

	contrast_slider = new QSlider(Qt::Horizontal);
	contrast_slider->setMinimum(1);
	contrast_slider->setMaximum(255);
	contrast_slider->setSingleStep(1);

	contrast_label = new QLabel;
	preemph_edit = new QLineEdit;

	layout->addWidget(wide_button);
	layout->addWidget(narrow_button);
	custom_layout->addWidget(custom_button);
	layout->addLayout(custom_layout);
	layout->addWidget(custom_edit);
	layout->addWidget(new QLabel("Frequency range (Hz):"));
	layout->addWidget(range_edit);
	layout->addWidget(new QLabel(tr("Window type:")));
	layout->addWidget(window_box);
	layout->addWidget(contrast_label);
	layout->addWidget(contrast_slider);
	layout->addWidget(new QLabel(QString::fromUtf8("Pre-emphasis threshold (Hz):")));
	layout->addWidget(preemph_edit);

	auto hl = new QHBoxLayout;
	auto reset_button = new QPushButton("Reset");
	auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	hl->addWidget(reset_button);
	hl->addStretch();
	hl->addWidget(button_box);
	layout->addLayout(hl);
	setLayout(layout);

	displayValues();

	connect(button_box, &QDialogButtonBox::accepted, this, &SpectrogramSettings::validate);
    connect(button_box, &QDialogButtonBox::rejected, this, &SpectrogramSettings::reject);
	connect(reset_button, &QPushButton::clicked, this, &SpectrogramSettings::reset);
    connect(wide_button, &QRadioButton::pressed, this, &SpectrogramSettings::disableCustomWindow);
	connect(narrow_button, &QRadioButton::pressed, this, &SpectrogramSettings::disableCustomWindow);
	connect(custom_button, &QRadioButton::pressed, this, &SpectrogramSettings::enableCustomWindow);
	connect(contrast_slider, &QSlider::valueChanged, [this](bool) { setContrastLabel(); });
}

void SpectrogramSettings::enableCustomWindow()
{
	custom_edit->setEnabled(true);
}

void SpectrogramSettings::disableCustomWindow()
{
	custom_edit->setEnabled(false);
}

void SpectrogramSettings::validate()
{
	bool ok;
	String category("spectrogram");

	double window_size = 0.005;
	if (narrow_button->isChecked())
		window_size = 0.025;
	else if (custom_button->isChecked())
	{
		auto value = custom_edit->text().toInt(&ok);
		if (!ok || value <= 0)
		{
			QMessageBox::critical(this, tr("Invalid setting"), tr("Invalid window size"));
			return;
		}
		window_size = double(value) / 1000;
	}
	Settings::set_value(runtime, category, "window_size", window_size);

	{
		int value = range_edit->text().toInt(&ok);
		if (!ok || value <= 0) {
			QMessageBox::critical(this, tr("Invalid setting"), tr("Invalid frequency range"));
			return;
		}
		Settings::set_value(runtime, category, "frequency_range", value);
	}

	String window_type = window_box->currentText();
	Settings::set_value(runtime, category, "window_type", window_type);

	Settings::set_value(runtime, category, "dynamic_range", contrast_slider->value());

	double threshold = preemph_edit->text().toDouble(&ok);
	if (!ok || threshold < 0)
	{
		QMessageBox::critical(this, tr("Invalid setting"), tr("Invalid pre-emphasis threshold"));
		return;
	}
	Settings::set_value(runtime, category, "preemphasis_threshold", threshold);

	accept();
}

void SpectrogramSettings::setContrastLabel()
{
	QString label = QString("Dynamic range: %1 dB").arg(contrast_slider->value());
	contrast_label->setText(label);
}

void SpectrogramSettings::displayValues()
{
	String category("spectrogram");

	auto range = (int) Settings::get_number(runtime, category, "dynamic_range");
	contrast_slider->setValue(range);

	auto frange = (int) Settings::get_number(runtime, category, "frequency_range");
	range_edit->setText(QString::number(frange));

	double window_size = Settings::get_number(runtime, category, "window_size");

	if (window_size == 0.005)
	{
		disableCustomWindow();
		wide_button->setChecked(true);
	}
	else if (window_size == 0.025)
	{
		disableCustomWindow();
		narrow_button->setChecked(true);
	}
	else
	{
		custom_button->setChecked(true);
		enableCustomWindow();
		custom_edit->setText(QString::number(window_size, 'g', 4));
	}

	auto window_type = Settings::get_string(runtime, category, "window_type");

	if (window_type == "Bartlett")
		window_box->setCurrentIndex(0);
	else if (window_type == "Blackman")
		window_box->setCurrentIndex(1);
	else if (window_type == "Gaussian")
		window_box->setCurrentIndex(2);
	else if (window_type == "Hamming")
		window_box->setCurrentIndex(3);
	else if (window_type == "Hann")
		window_box->setCurrentIndex(4);
	else if (window_type == "Rectangular")
		window_box->setCurrentIndex(5);
	else
		throw error("Invalid window type \"%\" in spectrogram settings");

	double threshold = Settings::get_number(runtime, category, "preemphasis_threshold");
	preemph_edit->setText(QString::number(threshold));

	setContrastLabel();
}

void SpectrogramSettings::reset(bool)
{
	run_script(runtime, reset_spectrogram_settings);
	displayValues();
}

} // namespace phonometrica
