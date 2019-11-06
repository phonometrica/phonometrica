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
 * Created: 07/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: settings for spectrogram.                                                                                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SPECTROGRAM_SETTINGS_HPP
#define PHONOMETRICA_SPECTROGRAM_SETTINGS_HPP

#include <QDialog>
#include <QSlider>
#include <QRadioButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>

namespace phonometrica {

class Runtime;

class SpectrogramSettings final : public QDialog
{
	Q_OBJECT

public:

	SpectrogramSettings(Runtime &rt, QWidget *parent = nullptr);


private slots:

	void enableCustomWindow();

	void disableCustomWindow();

	void validate();

	void reset(bool);

private:

	void setContrastLabel();

	void displayValues();

	Runtime &runtime;

	QSlider *contrast_slider;

	QRadioButton *wide_button, *narrow_button, *custom_button;

	QLineEdit *custom_edit, *range_edit, *preemph_edit;

	QComboBox *window_box;

	QLabel *contrast_label;
};

} // namespace phonometrica



#endif // PHONOMETRICA_SPECTROGRAM_SETTINGS_HPP
