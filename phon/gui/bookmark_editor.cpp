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