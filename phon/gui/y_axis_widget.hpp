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
 * Purpose: Display Y-axis information in sound and annotation views.                                                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_Y_AXIS_WIDGET_HPP
#define PHONOMETRICA_Y_AXIS_WIDGET_HPP

#include <set>
#include <QWidget>
#include <phon/array.hpp>
#include <phon/gui/speech_widget.hpp>

namespace phonometrica {

class SpeechPlot;

class YAxisWidget final : public QWidget
{
    Q_OBJECT

public:

    YAxisWidget(QWidget *parent = nullptr);

    void addWidget(SpeechWidget *widget);

    void removeWidget(SpeechWidget *widget);

    void clearWidgets();

public slots:

    void refresh();

protected:

    void paintEvent(QPaintEvent *) override;

private:

    // Non-owning references to visible widgets (in vertical order)
    std::set<SpeechWidget*> widgets;

};

} // namespace phonometrica

#endif // PHONOMETRICA_Y_AXIS_WIDGET_HPP
