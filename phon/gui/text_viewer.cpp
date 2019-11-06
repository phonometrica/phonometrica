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
 * Created: 16/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLayout>
#include <QPushButton>
#include <QTextEdit>
#include <phon/file.hpp>
#include <phon/gui/text_viewer.hpp>

namespace phonometrica {

TextViewer::TextViewer(const String &path, QString title, QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle(title);
	setMinimumWidth(500);
	auto layout = new QVBoxLayout;
	auto edit = new QTextEdit;
	auto text = File::read_all(path);
	edit->setText(text);
	layout->addWidget(edit);
	auto btn = new QPushButton(tr("Close"));
	auto hl = new QHBoxLayout;
	hl->addStretch(1);
	hl->addWidget(btn);
	layout->addLayout(hl);
	setLayout(layout);

	connect(btn, &QPushButton::clicked, this, &TextViewer::accept);
}
} // namespace phonometrica
