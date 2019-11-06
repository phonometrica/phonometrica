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
 * Created: 23/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QDebug>
#include <phon/gui/y_axis_widget.hpp>
#include <phon/gui/speech_plot.hpp>

namespace phonometrica {

YAxisWidget::YAxisWidget(QWidget *parent) :
    QWidget(parent)
{

}

void YAxisWidget::addWidget(SpeechWidget *widget)
{
    widgets.insert(widget);
}

void YAxisWidget::removeWidget(SpeechWidget *widget)
{
	widgets.erase(widget);
}

void YAxisWidget::clearWidgets()
{
    widgets.clear();
}

void YAxisWidget::paintEvent(QPaintEvent *)
{
    for (auto widget : widgets)
    {
    	widget->setYAxisItemVisible(widget->isVisible());

        if (widget->isVisible())
        {
            // Get absolute vertical bounds
            auto geom = widget->geometry();
            int y1 = geom.y();
            int y2 = geom.y() + widget->height();

            widget->drawYAxis(this, y1, y2);
        }
    }
}

void YAxisWidget::refresh()
{
    repaint();
}

} // namespace phonometrica
