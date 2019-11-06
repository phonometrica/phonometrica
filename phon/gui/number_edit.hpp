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
 * Created: 05/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: similar to a QTextEdit, but for a number.                                                                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_NUMBER_EDIT_HPP
#define PHONOMETRICA_NUMBER_EDIT_HPP

#include <functional>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <phon/string.hpp>
#include <phon/gui/line_edit.hpp>

namespace phonometrica {

class NumberEdit : public QWidget
{
public:

	explicit NumberEdit(String desc);

	std::function<bool(double)> functor() const;

	double value() const;

	bool hasValue() const;

	String description() const; // store useful info (e.g. category for numeric properties)

private:

	LineEdit *num_edit;
	QComboBox *op_box; // operator
	String desc;
};

} // namespace phonometrica

#endif // PHONOMETRICA_NUMBER_EDIT_HPP
