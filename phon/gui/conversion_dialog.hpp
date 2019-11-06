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
 * Created: 21/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: dialog for sound resampling and format conversion.                                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CONVERSION_DIALOG_HPP
#define PHONOMETRICA_CONVERSION_DIALOG_HPP

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <phon/gui/file_selector.hpp>

namespace phonometrica {

class ConversionDialog : public QDialog
{
	Q_OBJECT

public:

	ConversionDialog(int sample_rate, QWidget *parent = nullptr);

	QString format() const;

	String path() const;

	int sample_rate() const { return Fs; }

private slots:

	void validate();

private:

	FileSelector *file_selector;

	QComboBox *format_box;

	QLineEdit *fs_line;

	int Fs = 0;
};

} // namespace phonometrica



#endif //PHONOMETRICA_CONVERSION_DIALOG_HPP
