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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/line_edit.hpp>

namespace phonometrica {

LineEdit::LineEdit(const QString &defaultText) : QLineEdit()
{
	this->defaultText = defaultText;
	setDefaultText();
}

QString LineEdit::text() const
{
	QString txt = QLineEdit::text();

	if (txt == defaultText)
		return "";
	else
		return txt;
}

void LineEdit::setDefaultText()
{
	setText(defaultText);
	setStyleSheet("QLineEdit { color: gray; font-style: italic }");
}

void LineEdit::focusInEvent(QFocusEvent *event)
{
	if (text() == "")
	{
		setText(""); // make sure text is not default text
		setStyleSheet("QLineEdit { }");
	}

	QLineEdit::focusInEvent(event);
}

void LineEdit::focusOutEvent(QFocusEvent *event)
{
	if (text() == "")
		setDefaultText();

	QLineEdit::focusOutEvent(event);
}

} // namespace phonometrica