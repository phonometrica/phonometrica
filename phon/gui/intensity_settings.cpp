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

#include <QLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <phon/runtime/runtime.hpp>
#include <phon/application/settings.hpp>
#include <phon/include/reset_intensity_settings_phon.hpp>
#include <phon/gui/intensity_settings.hpp>

namespace phonometrica {

IntensitySettings::IntensitySettings(Runtime &rt, QWidget *parent) : 
	QDialog(parent), runtime(rt)
{
	setWindowTitle("Change intensity settings...");
	setMinimumWidth(300);

	min_edit = new QLineEdit;
	max_edit = new QLineEdit;
	step_edit = new QLineEdit;

	auto layout = new QVBoxLayout;
	layout->addWidget(new QLabel("Minimum intensity (dB):"));
	layout->addWidget(min_edit);
	layout->addWidget(new QLabel("Maximum intensity (dB):"));
	layout->addWidget(max_edit);
	layout->addWidget(new QLabel("Time step (seconds):"));
	layout->addWidget(step_edit);

	layout->addSpacing(10);

	auto hl = new QHBoxLayout;
	auto reset_button = new QPushButton("Reset");
	auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	hl->addWidget(reset_button);
	hl->addStretch();
	hl->addWidget(button_box);
	layout->addLayout(hl);
	setLayout(layout);
	displayValues();

	connect(button_box, &QDialogButtonBox::accepted, this, &IntensitySettings::validate);
	connect(button_box, &QDialogButtonBox::rejected, this, &IntensitySettings::reject);
	connect(reset_button, &QPushButton::clicked, this, &IntensitySettings::reset);
}

void IntensitySettings::validate()
{
	String category("intensity");
	bool ok;

	double value = min_edit->text().toDouble(&ok);
	if (!ok)
	{
		QMessageBox::critical(this, "Error", "Invalid minimum intensity");
		return;
	}
	Settings::set_value(runtime, category, "minimum_intensity", value);

	value = max_edit->text().toDouble(&ok);
	if (!ok)
	{
		QMessageBox::critical(this, "Error", "Invalid maximum intensity");
		return;
	}
	Settings::set_value(runtime, category, "maximum_intensity", value);

	value = step_edit->text().toDouble(&ok);
	if (!ok || value <= 0)
	{
		QMessageBox::critical(this, "Error", "Invalid time step");
		return;
	}
	Settings::set_value(runtime, category, "time_step", value);


	accept();
}

void IntensitySettings::displayValues()
{
	String category("intensity");
	auto minimum = Settings::get_number(runtime, category, "minimum_intensity");
	min_edit->setText(QString::number(minimum));

	auto maximum = Settings::get_number(runtime, category, "maximum_intensity");
	max_edit->setText(QString::number(maximum));

	auto step = Settings::get_number(runtime, category, "time_step");
	step_edit->setText(QString::number(step));
}

void IntensitySettings::reset(bool)
{
	run_script(runtime, reset_intensity_settings);
    displayValues();
}
} // namespace phonometrica
