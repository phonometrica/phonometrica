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
 * Created: 18/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: scrollbar that displays the whole sound file in a sound view or annotation view.                          *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_WAVE_BAR_HPP
#define PHONOMETRICA_WAVE_BAR_HPP

#include <utility>
#include <QWidget>
#include <QPainter>
#include <phon/application/audio_data.hpp>

namespace phonometrica {

class WaveBar final : public QWidget
{
    Q_OBJECT

public:

    WaveBar(const std::shared_ptr<AudioData> &data, QWidget *parent = nullptr);

    double magnitude() const;

    void setInitialSelection();

signals:

    void selectionStarted();

    // Update the plots when a user selects a new portion of the file.
    void timeSelection(double, double);

    // Update the sound zoom when a plot updates the scrollbar.
    void updatedXAxisSelection(double, double);

public slots:

    void setTimeSelection(double from, double to);

protected:

    void paintEvent(QPaintEvent* event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

private:

    int sampleToYPos(sample_t s) const;

    std::shared_ptr<AudioData> m_data;

    //Array<int> m_cache;

    double sampleToXPos(intptr_t s);

    intptr_t xPosToSample(double x);

    double timeToXPos(double t);

    double xPosToTime(double x);

    void notifySelection();

    void makePath(QPainterPath &path);

    // We cache the sinusoidal so that we don't need to recalculate it on each redraw if the geometry doesn't change.
    QPainterPath cached_path;

    // Cache the width of the plot
    int cached_width = 0;

    double from, to; // buffer x axis positions when mouse is pressed

    // Normalization constants
    double raw_magnitude;
};

} // namespace phonometrica


#endif //PHONOMETRICA_WAVE_BAR_HPP
