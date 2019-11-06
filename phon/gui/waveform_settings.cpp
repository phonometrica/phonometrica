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
 * Created: 10/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <phon/runtime/runtime.hpp>
#include <phon/application/settings.hpp>
#include <phon/include/reset_waveform_settings_phon.hpp>
#include <phon/gui/waveform_settings.hpp>

namespace phonometrica {

WaveformSettings::WaveformSettings(Runtime &rt, QWidget *parent) :
	QDialog(parent), runtime(rt)
{
    setWindowTitle("Change waveform settings...");
    setMinimumWidth(300);

	auto layout = new QVBoxLayout;
	layout->addWidget(new QLabel(tr("Scaling:")));
	scaling_box = new QComboBox;
	scaling_box->addItem(tr("Global magnitude"));
	scaling_box->addItem(tr("Local magnitude"));
	scaling_box->addItem(tr("Fixed magnitude"));
	layout->addWidget(scaling_box);

	magnitude_edit = new QLineEdit;
	layout->addWidget(magnitude_edit);

	auto hl = new QHBoxLayout;
	auto reset_button = new QPushButton("Reset");
	auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	hl->addWidget(reset_button);
	hl->addStretch();
	hl->addWidget(button_box);
	layout->addLayout(hl);

	setLayout(layout);
	displayValues();

	connect(button_box, &QDialogButtonBox::accepted, this, &WaveformSettings::validate);
    connect(button_box, &QDialogButtonBox::rejected, this, &WaveformSettings::reject);
    connect(reset_button, &QPushButton::clicked, this, &WaveformSettings::reset);

    connect(scaling_box, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int i) {
    	magnitude_edit->setEnabled(i==2);
    	if (i!=2) magnitude_edit->setText("1.0");
    });
}

void WaveformSettings::validate()
{
	String category("waveform");
	String scaling;

	switch (scaling_box->currentIndex())
	{
		case 0:
			Settings::set_value(runtime, category, "scaling", "global");
			break;
		case 1:
			Settings::set_value(runtime, category, "scaling", "local");
			break;
		default:
		{
			bool ok;
			auto mag = magnitude_edit->text().toDouble(&ok);
			if (!ok || mag <= 0 || mag > 1.0)
			{
				QMessageBox::critical(this, tr("Invalid settings"), tr("Invalid magnitude"));
				return;
			}
			Settings::set_value(runtime, category, "scaling", "fixed");
			Settings::set_value(runtime, category, "magnitude", mag);
		}
	}

	accept();
}

void WaveformSettings::reset(bool)
{
	run_script(runtime, reset_waveform_settings);
    displayValues();
}

void WaveformSettings::displayValues()
{
	String category("waveform");

	auto mag = Settings::get_number(runtime, category, "magnitude");
	magnitude_edit->setText(QString::number(mag, 'g', 4));

	auto scaling = Settings::get_string(runtime, category, "scaling");

	if (scaling == "global")
	{
		scaling_box->setCurrentIndex(0);
		magnitude_edit->setEnabled(false);
	}
	else if (scaling == "local")
	{
		scaling_box->setCurrentIndex(1);
		magnitude_edit->setEnabled(false);
	}
	else if (scaling == "fixed")
	{
		scaling_box->setCurrentIndex(2);
		magnitude_edit->setEnabled(true);
	}
	else
	{
		throw error("Unrecognized waveform scaling option: %", scaling);
	}
}
} // namespace phonometrica
