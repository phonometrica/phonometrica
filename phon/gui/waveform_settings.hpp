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
 * Purpose: settings for the waveform plot.                                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_WAVEFORM_SETTINGS_HPP
#define PHONOMETRICA_WAVEFORM_SETTINGS_HPP

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>

namespace phonometrica {

class Runtime;

class WaveformSettings final : public QDialog
{
	Q_OBJECT

public:

	WaveformSettings(Runtime &rt, QWidget *parent = nullptr);

private slots:

	void validate();

	void reset(bool);

private:

	void displayValues();

	Runtime &runtime;

	QComboBox *scaling_box;

	QLineEdit *magnitude_edit;
};

} // namespace phonometrica

#endif // PHONOMETRICA_WAVEFORM_SETTINGS_HPP
