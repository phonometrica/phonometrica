/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
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