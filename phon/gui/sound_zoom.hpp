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
 * Purpose: connects the sound scrollbar to the other sound plots in a sound view or annotation view.                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SOUND_ZOOM_HPP
#define PHONOMETRICA_SOUND_ZOOM_HPP

#include <QWidget>

namespace phonometrica {

class SoundZoom : public QWidget
{
    Q_OBJECT

public:

    SoundZoom(QWidget *parent);

public slots:

    void setXAxisSelection(double from, double to);

    void hideSelection();

protected:

    void paintEvent(QPaintEvent* event) override;

private:

    double from, to;

};

} // namespace phonometrica

#endif // PHONOMETRICA_SOUND_ZOOM_HPP
