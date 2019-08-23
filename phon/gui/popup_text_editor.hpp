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
 * Created: 24/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: edit the text of an event in an annotation view.                                                          *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_POPUP_TEXT_EDITOR_HPP
#define PHONOMETRICA_POPUP_TEXT_EDITOR_HPP

#include <QDialog>
#include <phon/gui/event_line.hpp>

namespace phonometrica {

class PopupTextEditor final : public QDialog
{
    Q_OBJECT

public:

    PopupTextEditor(const QString &text, QPoint pos, QWidget *parent = nullptr);

    QString text() const { return edit->toPlainText(); }

    int yshift() const;

protected:

    void paintEvent(QPaintEvent *event) override;



private:

    EventLine *edit;

    int ypos;
};

} // namespace phonometrica

#endif // PHONOMETRICA_POPUP_TEXT_EDITOR_HPP
