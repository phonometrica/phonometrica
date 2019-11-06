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
 * Created: 03/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: import dialog for CSV files.                                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CSV_IMPORT_DIALOG_HPP
#define PHONOMETRICA_CSV_IMPORT_DIALOG_HPP

#include <QDialog>
#include <QLineEdit>
#include <QRadioButton>
#include <phon/string.hpp>

namespace phonometrica {

class Runtime;

class CsvImportDialog : public QDialog
{
	Q_OBJECT

public:

	CsvImportDialog(QWidget *parent, Runtime &rt);

	String separator() const;

	String path() const;

private slots:

	void setPath(bool);

private:

	Runtime &runtime;

	QLineEdit *path_edit;

	QRadioButton *semicolon_button, *comma_button, *tab_button;
};

} // namespace phonometrica

#endif // PHONOMETRICA_CSV_IMPORT_DIALOG_HPP
