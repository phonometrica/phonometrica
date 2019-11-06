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
 * Purpose: adjust pitch settings from a sound or annotation view.                                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PITCH_SETTINGS_HPP
#define PHONOMETRICA_PITCH_SETTINGS_HPP

#include <QDialog>
#include <QLineEdit>
#include <QSlider>
#include <QLabel>

namespace phonometrica {

class Runtime;

class PitchSettings final : public QDialog
{
    Q_OBJECT

public:

    PitchSettings(Runtime &rt, QWidget *parent = nullptr);

    double min_pitch, max_pitch, time_step, voicing_threshold;

private slots:

    void validate();

    void reset(bool);

    void updatePitchThreshold(int value);

private:

    void displayValues();

    Runtime &runtime;

    QLineEdit *min_edit, *max_edit, *step_edit;

    QLabel *threshold_label;

    QSlider *slider;
};

} // namespace phonometrica

#endif // PHONOMETRICA_PITCH_SETTINGS_HPP
