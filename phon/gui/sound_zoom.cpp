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
 * Created: 18/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QPainter>
#include <phon/gui/sound_zoom.hpp>
#include "sound_zoom.hpp"


namespace phonometrica {

SoundZoom::SoundZoom(QWidget *parent) :
    QWidget(parent)
{
    setFixedHeight(40);
    from = to = 0.0;
}

void SoundZoom::setXAxisSelection(double from, double to)
{
    this->from = from;
    this->to = to;
    this->repaint();
}

void SoundZoom::paintEvent(QPaintEvent *event)
{
    if (from == 0.0 && to == 0.0) {
        return;
    }

    QPainterPath path;
    path.moveTo(QPointF(0.0, 0.0));
    path.lineTo(QPointF(from, height()));
    path.lineTo(QPointF(to, height()));
    path.lineTo(QPointF(width(), 0.0));
    path.lineTo(QPointF(0.0, 0.0));

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor col(Qt::blue);
    col = col.lighter(160);
    col.setAlpha(60);
    painter.fillPath(path, col);

    QWidget::paintEvent(event);
}

void SoundZoom::hideSelection()
{
    from = to = 0.0;
    repaint();
}
} // namespace phonometrica