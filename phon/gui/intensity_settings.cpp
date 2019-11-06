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
