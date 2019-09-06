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

#include <QLabel>
#include <phon/gui/number_edit.hpp>

namespace phonometrica {

NumberEdit::NumberEdit(QString desc) : QWidget()
{
	auto layout = new QVBoxLayout;
	this->desc = desc;
	op_box = new QComboBox;
	num_edit = new LineEdit(tr("numeric value"));

	QStringList ops;
	ops << "equal" << "not equal" << "less than" << "less than or equal" << "greater than" << "greater than or equal";

	for (auto &op: ops) {
		op_box->addItem(op);
	}

	layout->addWidget(new QLabel("<b>" + desc + "</b>"));
	layout->addWidget(op_box);
	layout->addWidget(num_edit);
	layout->addStretch(1);
	setLayout(layout);
	op_box->setFocusProxy(num_edit);
}

QString NumberEdit::description() const
{
	return desc;
}

std::function<bool(double)> NumberEdit::get_functor() const
{
	double val = this->value();

	switch (op_box->currentIndex())
	{
		case 0:
			return [=](double x) { return x == val; };
		case 1:
			return [=](double x) { return x != val; };
		case 2:
			return [=](double x) { return x < val; };
		case 3:
			return [=](double x) { return x <= val; };
		case 4:
			return [=](double x) { return x > val; };
		default:
			return [=](double x) { return x >= val; };
	}
}

double NumberEdit::value() const
{
	return num_edit->text().toDouble();
}

} // namespace phonometrica