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
	step_edit = new QLineEdit;
	npole_edit = new QLineEdit;
	fs_edit = new QLineEdit;

	auto layout = new QVBoxLayout;
	layout->addWidget(new QLabel(tr("Number of formants:")));
	layout->addWidget(nformant_edit);
	layout->addWidget(new QLabel(tr("Maximum frequency (Hz):")));
	layout->addWidget(fs_edit);
	layout->addWidget(new QLabel(tr("Window length (ms):")));
	layout->addWidget(window_edit);
	layout->addWidget(new QLabel(tr("Time step (s)")));
	layout->addWidget(step_edit);
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

	auto step = Settings::get_number(runtime, category, "time_step");
	step_edit->setText(QString::number(step));

	auto npole = (int) Settings::get_number(runtime, category, "lpc_order");
	npole_edit->setText(QString::number(npole));

	auto fs = (int) Settings::get_number(runtime, category, "max_frequency");
	fs_edit->setText(QString::number(fs));
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

	auto step = step_edit->text().toDouble(&ok);
	if (!ok || step <= 0) {
		QMessageBox::critical(this, tr("Invalid setting"), tr("Invalid time step"));
		return;
	}
	Settings::set_value(runtime, category, "time_step", step);

	auto npole = npole_edit->text().toInt(&ok);
	if (!ok || npole <= nformant) {
		QMessageBox::critical(this, tr("Invalid setting"), tr("Invalid LPC order"));
		return;
	}
	Settings::set_value(runtime, category, "lpc_order", npole);

	auto fs = fs_edit->text().toInt(&ok);
	if (!ok || fs <= 0) {
		QMessageBox::critical(this, tr("Invalid setting"), tr("Invalid down-sampling frequency"));
		return;
	}
	Settings::set_value(runtime, category, "max_frequency", fs);

	accept();
}

void FormantSettings::reset(bool)
{
	run_script(runtime, reset_formants_settings);
	displayValues();
}
} // namespace phonometrica
