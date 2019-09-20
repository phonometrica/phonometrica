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
 * Created: 29/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <phon/gui/pitch_settings.hpp>
#include <phon/application/settings.hpp>
#include <phon/runtime/runtime.hpp>

namespace phonometrica {

PitchSettings::PitchSettings(Runtime &rt, QWidget *parent) :
    QDialog(parent), rt(rt)
{
    setWindowTitle("Change pitch settings...");
    setMinimumWidth(300);

    min_edit = new QLineEdit;
    max_edit = new QLineEdit;
    step_edit = new QLineEdit;

    auto layout = new QVBoxLayout;
    layout->addWidget(new QLabel("Minimum pitch (Hz):"));
    layout->addWidget(min_edit);
    layout->addWidget(new QLabel("Maximum pitch (Hz):"));
    layout->addWidget(max_edit);
    layout->addWidget(new QLabel("Time step (seconds):"));
    layout->addWidget(step_edit);
    layout->addWidget(new QLabel("Voicing threshold:"));
    auto voicing_layout = new QHBoxLayout;
    threshold_label = new QLabel;
    slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(0);
    slider->setMaximum(30);
    slider->setTickInterval(1);
    voicing_layout->addWidget(threshold_label);
    voicing_layout->addWidget(slider);
    layout->addLayout(voicing_layout);
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
    connect(slider, &QSlider::valueChanged, this, &PitchSettings::updatePitchThreshold);
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
    Settings::set_value(rt, category, "minimum_pitch", value);

    value = max_edit->text().toDouble(&ok);
    if (!ok)
    {
        QMessageBox::critical(this, "Error", "Invalid maximum pitch");
        return;
    }
    Settings::set_value(rt, category, "maximum_pitch", value);

    value = step_edit->text().toDouble(&ok);
    if (!ok || value <= 0)
    {
        QMessageBox::critical(this, "Error", "Invalid time step");
        return;
    }
    Settings::set_value(rt, category, "time_step", value);

    value = threshold_label->text().toDouble(&ok);
    if (!ok || value > 0.5 || value < 0.2)
    {
    	// Should never happen
        QMessageBox::critical(this, "Error", "Invalid voicing threshold");
        return;
    }
    Settings::set_value(rt, category, "voicing_threshold", value);

    accept();
}

void PitchSettings::reset(bool)
{
    rt.do_string(R"__(
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
    auto minimum = Settings::get_number(rt, category, "minimum_pitch");
    min_edit->setText(QString::number(minimum));

    auto maximum = Settings::get_number(rt, category, "maximum_pitch");
    max_edit->setText(QString::number(maximum));

    auto step = Settings::get_number(rt, category, "time_step");
    step_edit->setText(QString::number(step));

    auto thres = Settings::get_number(rt, category, "voicing_threshold");
    int value = int((thres - 0.2) * 100);
    slider->setValue(value);
    threshold_label->setText(QString::number(thres));
}

void PitchSettings::updatePitchThreshold(int value)
{
	double thres = 0.2 + (double(value) / 100);
	threshold_label->setText(QString::number(thres, 'g', 2));
}
} // namespace phonometrica
