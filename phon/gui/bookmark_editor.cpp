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
 * Created: 14/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QMessageBox>
#include <phon/gui/bookmark_editor.hpp>

namespace phonometrica {

BookmarkEditor::BookmarkEditor(QWidget *parent) : QDialog(parent)
{
    QLabel *name_label = new QLabel(tr("Title:"));
	title_line = new QLineEdit;

    QLabel *content_label = new QLabel(tr("Notes:"));
	notes_edit = new QTextEdit;

	ok_button = new QPushButton(tr("&OK"));
	cancel_button = new QPushButton(tr("&Cancel"));

    QHBoxLayout *button_layout = new QHBoxLayout;
    button_layout->addStretch();
    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->addWidget(name_label);
    main_layout->addWidget(title_line);
    main_layout->addWidget(content_label);
    main_layout->addWidget(notes_edit);
    main_layout->addLayout(button_layout);

    setLayout(main_layout);
    setWindowTitle(tr("Bookmark Editor"));

	connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
}

QString BookmarkEditor::title() const
{
	return title_line->text();
}

QString BookmarkEditor::notes() const
{
	return notes_edit->toPlainText();
}


} // namespace phonometrica