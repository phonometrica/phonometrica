/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 05/09/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: same as a QTextEdit, but for a Boolean value.                                                             *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_BOOLEAN_EDIT_HPP
#define PHONOMETRICA_BOOLEAN_EDIT_HPP

#include <optional>
#include <QLayout>
#include <QComboBox>

namespace phonometrica {

class BooleanEdit : public QHBoxLayout
{
public:

	explicit BooleanEdit(QString desc);

	std::optional<bool> value() const;

	QString description() const; // store useful info (e.g. category for numeric properties)

	void show();

	void hide();

private:

	QComboBox *combo_box;

	QString desc;
};

} // namespace phonometrica

#endif // PHONOMETRICA_BOOLEAN_EDIT_HPP
