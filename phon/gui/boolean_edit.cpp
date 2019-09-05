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

#include <phon/gui/boolean_edit.hpp>

namespace phonometrica {

BooleanEdit::BooleanEdit(QString desc) : QHBoxLayout()
{
	this->desc = std::move(desc);
	combo_box = new QComboBox;
	combo_box->addItem("any");
	combo_box->addItem("true");
	combo_box->addItem("false");

	this->addWidget(combo_box);
	this->addStretch(1);
}

QString BooleanEdit::description() const
{
	return desc;
}

std::optional<bool> BooleanEdit::value() const
{
	switch (combo_box->currentIndex())
	{
		case 1:
			return true;
		case 2:
			return false;
		default:
			return std::optional<bool>();
	}
}

void BooleanEdit::show()
{
	combo_box->show();
}

void BooleanEdit::hide()
{
	combo_box->hide();
}

} // namespace phonometrica
