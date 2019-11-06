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
 * Purpose: editor to create bookmarks.                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_BOOKMARK_EDITOR_HPP
#define PHONOMETRICA_BOOKMARK_EDITOR_HPP


#include <memory>
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>

namespace phonometrica {

class BookmarkEditor final : public QDialog
{
    Q_OBJECT

public:

    BookmarkEditor(QWidget *parent = nullptr);

    QString title() const;

    QString notes() const;

private:

    QLineEdit   *title_line;
    QTextEdit   *notes_edit;
    QPushButton *ok_button, *cancel_button;
};

} // namespace phonometrica

#endif // PHONOMETRICA_BOOKMARK_EDITOR_HPP
