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
 * Created: 15/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QMessageBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLayout>
#include <QLabel>
#include <phon/definitions.hpp>
#include <phon/include/reset_formants_settings_phon.hpp>
#include <phon/gui/formant_settings.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

FormantSettings::FormantSettings(Runtime &rt, QWidget *parent) :
	QDialog(parent), runtime(rt)
{
	setWindowTitle("Formant settings...");
	setMinimumWidth(300);
	nformant_edit = new QLineEdit;
	window_edit = new QLineEdit;
	npole_edit = new QLineEdit;
	max_freq_edit = new QLineEdit;
	max_bw_edit = new QLineEdit;

	auto layout = new QVBoxLayout;
	layout->addWidget(new QLabel(tr("Number of formants:")));
	layout->addWidget(nformant_edit);
	layout->addWidget(new QLabel(tr("Maximum frequency (Hz):")));
	layout->addWidget(max_freq_edit);
	layout->addWidget(new QLabel(tr("Maximum bandwidth (Hz):")));
	layout->addWidget(max_bw_edit);
	layout->addWidget(new QLabel(tr("Window length (ms):")));
	layout->addWidget(window_edit);
	layout->addWidget(new QLabel(tr("LPC order:")));
	layout->addWidget(npole_edit);

	auto hl = new QHBoxLayout;
	auto reset_button = new QPushButton("Reset");
	auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	hl->addWidget(reset_button);
	hl->addStretch();
	hl->addWidget(button_box);
	layout->addLayout(hl);
	setLayout(layout);

	displayValues();
	connect(button_box, &QDialogButtonBox::accepted, this, &FormantSettings::validate);
    connect(button_box, &QDialogButtonBox::rejected, this, &FormantSettings::reject);
	connect(reset_button, &QPushButton::clicked, this, &FormantSettings::reset);
}

void FormantSettings::displayValues()
{
	String category("formants");

	auto nformant = (int) Settings::get_number(runtime, category, "number_of_formants");
	nformant_edit->setText(QString::number(nformant));

	auto len = Settings::get_number(runtime, category, "window_size");
	window_edit->setText(QString::number(len));

	auto npole = (int) Settings::get_number(runtime, category, "lpc_order");
	npole_edit->setText(QString::number(npole));

	auto fs = (int) Settings::get_number(runtime, category, "max_frequency");
	max_freq_edit->setText(QString::number(fs));

	auto bw = (int) Settings::get_number(runtime, category, "max_bandwidth");
	max_bw_edit->setText(QString::number(bw));
}

void FormantSettings::validate()
{
	bool ok;
	String category("formants");

	auto nformant = nformant_edit->text().toInt(&ok);
	if (!ok || nformant <= 0 || nformant > PHON_MAX_FORMANTS) {
		QMessageBox::critical(this, tr("Invalid setting"), tr("Invalid number of formants"));
		return;
	}
	Settings::set_value(runtime, category, "number_of_formants", nformant);

	auto len = window_edit->text().toDouble(&ok);
	if (!ok || len <= 0) {
		QMessageBox::critical(this, tr("Invalid setting"), tr("Invalid window length"));
		return;
	}
	Settings::set_value(runtime, category, "window_size", len);

	auto npole = npole_edit->text().toInt(&ok);
	if (!ok || npole <= nformant) {
		QMessageBox::critical(this, tr("Invalid setting"), tr("Invalid LPC order"));
		return;
	}
	Settings::set_value(runtime, category, "lpc_order", npole);

	auto fs = max_freq_edit->text().toInt(&ok);
	if (!ok || fs <= 0) {
		QMessageBox::critical(this, tr("Invalid setting"), tr("Invalid maximum frequency"));
		return;
	}
	Settings::set_value(runtime, category, "max_frequency", fs);

	auto bw = max_bw_edit->text().toInt(&ok);
	if (!ok || bw <= 0) {
		QMessageBox::critical(this, tr("Invalid setting"), tr("Invalid maximum bandwidth"));
		return;
	}

	accept();
}

void FormantSettings::reset(bool)
{
	run_script(runtime, reset_formants_settings);
	displayValues();
}
} // namespace phonometrica
