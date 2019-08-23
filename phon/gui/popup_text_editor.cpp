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
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <QVBoxLayout>
#include <phon/gui/popup_text_editor.hpp>
#include "popup_text_editor.hpp"


namespace phonometrica {

PopupTextEditor::PopupTextEditor(const QString &text, QPoint pos, QWidget *parent) :
    QDialog(parent)
{
    auto layout = new QVBoxLayout;
    edit = new EventLine;
    edit->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(edit);
    setLayout(layout);
    edit->setText(text);
    int xdelta = 400;
    int ydelta = 50;
    int x = pos.x() - xdelta;
    ypos = pos.y() - ydelta*2 - 10;
    setGeometry(x, ypos, xdelta*2, ydelta*2);
    setContentsMargins(0, 0, 0, 0);
    setWindowFlags(Qt::Window|Qt::FramelessWindowHint);
    setWindowOpacity(0.95);

    connect(edit, &EventLine::accepted, this, &PopupTextEditor::accept);
}

void PopupTextEditor::paintEvent(QPaintEvent *event)
{
    QPalette pal = palette();
    auto col = QColor(Qt::blue).lighter(160);
    col.setAlpha(200);
    pal.setColor(QPalette::Background, col);
    setAutoFillBackground(true);
    setPalette(pal);

    QWidget::paintEvent(event);
}

int PopupTextEditor::yshift() const
{
    return y() - ypos;
}


} // namespace phonometrica