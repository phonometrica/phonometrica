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
 * Created: 20/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QToolButton>
#include <phon/gui/button_stylesheet.hpp>
#include <phon/gui/toolbar.hpp>

namespace phonometrica {

MacToolbar::MacToolbar(QWidget *parent)
{
	stylesheet = flat_button_stylesheet;
	layout = new QHBoxLayout;
	setLayout(layout);
	setFixedHeight(50);
	layout->setContentsMargins(0, 5, 0, 5);
	layout->setSpacing(0);
}

QAction *MacToolbar::addAction(const QIcon &icon, const QString &text)
{
	auto action = new QAction(icon, text);
	addAction(action);

	return action;
}

void MacToolbar::addWidget(QWidget *w)
{
	w->setFixedSize(40, 40);
	w->setStyleSheet(stylesheet);
	auto button = dynamic_cast<QToolButton*>(w);
	if (button) button->setIconSize(QSize(20, 20));

	layout->addWidget(w);
}

void MacToolbar::addStretch(int factor)
{
	layout->addStretch(factor);
}

void MacToolbar::addSpacing(int spacing)
{
	layout->addSpacing(spacing);
}

void MacToolbar::addAction(QAction *action)
{
	auto button = new QToolButton;
	button->setDefaultAction(action);
	addWidget(button);
}

void MacToolbar::addSeparator()
{
	layout->addWidget(new Separator);
}
} // namespace phonometrica