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
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <phon/gui/number_edit.hpp>

namespace phonometrica {

NumberEdit::NumberEdit(QString desc) : QHBoxLayout()
{
	this->desc = std::move(desc);
	op_box = new QComboBox;
	num_edit = new LineEdit(tr("numeric value"));

	QStringList ops;
	ops << "==" << "!=" << "<" << "<=" << ">" << ">=";

	for (auto &op: ops) {
		op_box->addItem(op);
	}

	this->addWidget(op_box);
	this->addWidget(num_edit);
	op_box->setFocusProxy(num_edit);
}

QString NumberEdit::description() const
{
	return desc;
}

QString NumberEdit::get_operator() const
{
	return op_box->currentText();
}

double NumberEdit::value() const
{
	return num_edit->text().toDouble();
}

void NumberEdit::show()
{
	num_edit->show();
	op_box->show();
}

void NumberEdit::hide()
{
	num_edit->hide();
	op_box->hide();
}

} // namespace phonometrica