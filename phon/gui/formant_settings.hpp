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
 * Purpose: settings dialog for formant analysis.                                                                      *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FORMANT_SETTINGS_HPP
#define PHONOMETRICA_FORMANT_SETTINGS_HPP

#include <QDialog>
#include <QLineEdit>

namespace phonometrica {

class Runtime;


class FormantSettings final : public QDialog
{
	Q_OBJECT

public:

	FormantSettings(Runtime &rt, QWidget *parent = nullptr);

private slots:

	void validate();

	void reset(bool);

private:

	void displayValues();

	Runtime &runtime;

	QLineEdit *nformant_edit, *window_edit, *npole_edit, *max_freq_edit, *max_bw_edit;
};

} // namespace phonometrica



#endif // PHONOMETRICA_FORMANT_SETTINGS_HPP
