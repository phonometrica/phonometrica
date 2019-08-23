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
 * Created: 29/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <phon/gui/pitch_settings.hpp>
#include <phon/application/settings.hpp>
#include <phon/runtime/environment.hpp>

namespace phonometrica {

PitchSettings::PitchSettings(Environment &env, QWidget *parent) :
    QDialog(parent), env(env)
{
    setWindowTitle("Change pitch settings...");
    setMinimumWidth(300);

    min_edit = new QLineEdit;
    max_edit = new QLineEdit;
    step_edit = new QLineEdit;
    threshold_edit = new QLineEdit;

    auto layout = new QVBoxLayout;
    layout->addWidget(new QLabel("Minimum pitch (Hz):"));
    layout->addWidget(min_edit);
    layout->addWidget(new QLabel("Maximum pitch (Hz):"));
    layout->addWidget(max_edit);
    layout->addWidget(new QLabel("Time step (seconds):"));
    layout->addWidget(step_edit);
    layout->addWidget(new QLabel("Voicing threshold (0.5 ≥ t ≥ 0.2):"));
    layout->addWidget(threshold_edit);
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

    connect(button_box, &QDialogButtonBox::accepted, this, &PitchSettings::validate);
    connect(button_box, &QDialogButtonBox::rejected, this, &PitchSettings::reject);
    connect(reset_button, &QPushButton::clicked, this, &PitchSettings::reset);
}

void PitchSettings::validate()
{
    bool ok;
    String category("pitch_tracking");

    double value = min_edit->text().toDouble(&ok);
    if (!ok)
    {
        QMessageBox::critical(this, "Error", "Invalid minimum pitch");
        return;
    }
    Settings::set_value(env, category, "minimum_pitch", value);

    value = max_edit->text().toDouble(&ok);
    if (!ok)
    {
        QMessageBox::critical(this, "Error", "Invalid maximum pitch");
        return;
    }
    Settings::set_value(env, category, "maximum_pitch", value);

    value = step_edit->text().toDouble(&ok);
    if (!ok || value <= 0)
    {
        QMessageBox::critical(this, "Error", "Invalid time step");
        return;
    }
    Settings::set_value(env, category, "time_step", value);

    value = threshold_edit->text().toDouble(&ok);
    if (!ok || value > 0.5 || value < 0.2)
    {
        QMessageBox::critical(this, "Error", "Invalid voicing threshold");
        return;
    }
    Settings::set_value(env, category, "voicing_threshold", value);

    accept();
}

void PitchSettings::reset(bool)
{
    env.do_string(R"__(
phon.settings.pitch_tracking = {
    minimum_pitch: 70,
    maximum_pitch: 500,
    time_step: 0.01,
    voicing_threshold: 0.25
}
)__");

    displayValues();
}

void PitchSettings::displayValues()
{
    String category("pitch_tracking");
    auto minimum = Settings::get_number(env, category, "minimum_pitch");
    min_edit->setText(QString::number(minimum));

    auto maximum = Settings::get_number(env, category, "maximum_pitch");
    max_edit->setText(QString::number(maximum));

    auto step = Settings::get_number(env, category, "time_step");
    step_edit->setText(QString::number(step));

    auto thres = Settings::get_number(env, category, "voicing_threshold");
    threshold_edit->setText(QString::number(thres));
}
} // namespace phonometrica
