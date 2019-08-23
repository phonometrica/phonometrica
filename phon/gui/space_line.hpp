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
 * Created: 29/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: line to separate widgets in speech views.                                                                 *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_SPACE_LINE_HPP
#define PHONOMETRICA_SPACE_LINE_HPP

#include <QPainter>
#include <QWidget>

namespace phonometrica {

class SpaceLine : public QWidget
{
public:

    SpaceLine() : QWidget()
    {
        setFixedHeight(1);
    }

    static QColor lineColor()
    {
        auto col = QColor(Qt::black);
        col.setAlpha(50);

        return col;
    }

protected:

    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);

        // Set background color
        QPalette pal = palette();
        auto col = lineColor();
        pal.setColor(QPalette::Background, col);
        setAutoFillBackground(true);
        setPalette(pal);
    }
};

} // namespace phonometrica

#endif // PHONOMETRICA_SPACE_LINE_HPP
