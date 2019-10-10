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
