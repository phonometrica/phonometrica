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
 * Purpose: custom line editor, which displays a default message when it is empty.                                    *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_LINE_EDIT_HPP
#define PHONOMETRICA_LINE_EDIT_HPP

#include <QLineEdit>

namespace phonometrica {

class LineEdit : public QLineEdit
{
	Q_OBJECT

public:
    LineEdit(const QString &defaultText);
	QString text() const;

public slots:
	void focusInEvent(QFocusEvent *event);
	void focusOutEvent(QFocusEvent *event);

private:
	QString defaultText;

    void setDefaultText();
};

} // namespace phonometrica

#endif // PHONOMETRICA_LINE_EDIT_HPP
