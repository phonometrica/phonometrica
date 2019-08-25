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
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <QDebug>
#include <phon/gui/waveform.hpp>
#include "waveform.hpp"


namespace phonometrica {

Waveform::Waveform(Runtime &rt, std::shared_ptr<AudioData> data, QWidget *parent) :
    SpeechPlot(rt, std::move(data), parent)
{

}

void Waveform::renderPlot(QPaintEvent *)
{
    QPainter painter(this);

    // Draw zero-crossing line.
    QPen pen;
    pen.setColor(Qt::blue);
    pen.setStyle(Qt::DotLine);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawLine(0, height()/2, width(), height()/2);

    if (needsRefresh())
    {
        drawWave();
    }
    else
    {
        painter.setRenderHint(QPainter::Antialiasing);
        QPen pen;
        pen.setColor(Qt::black);
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.drawPath(cached_path);
    }
}

bool Waveform::needsRefresh() const
{
    return SpeechPlot::needsRefresh() || cached_width != width() || cached_height != height() || cached_path.isEmpty();
}

void Waveform::drawWave()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setColor(Qt::black);
    painter.setPen(pen);

    // If the number of samples to display is greater than the number of pixels,
    // map several frames to one pixel. We find the maximum and minimum amplitudes
    // over the range of frames, and we draw a line from the previous minimum to the
    // current maximum, and from the current maximum to the current mimimum.

    auto first_sample = m_data->time_to_frame(window_start);
    auto last_sample = m_data->time_to_frame(window_end);
    auto sample_count = last_sample - first_sample + 1;
    auto raw_data = m_data->data() + first_sample - 1; // to base 0

    // TODO: smooth waveform drawing
    if (sample_count > this->width())
    {
        QPainterPath path;
        int x = 0;
        sample_t previous_value = 0;
        assert(window_end <= m_data->duration());

        // Subtract 1 to width so that the last pixel is assigned the left-over frames.
        auto frames_per_pixel = sample_count / (this->width()-1);

        auto maximum = (std::numeric_limits<sample_t>::min)();
        auto minimum = (std::numeric_limits<sample_t>::max)();

        for (intptr_t i = 1; i <= sample_count; i++)
        {
            auto sample = *raw_data++;

            if (sample < minimum) {
                minimum = sample;
            }
            if (sample > maximum) {
                maximum = sample;
            }

            if (i % frames_per_pixel == 0)
            {
                double y1 = sampleToHeight(maximum);
                double y2 = sampleToHeight(minimum);

                if (x == 0)
                {
                    path.moveTo(0.0, y1);
                }
                else
                {
                    path.lineTo(x, y1);
                }
                path.lineTo(x, y2);
                x++;

                // reset values
                maximum = (std::numeric_limits<sample_t>::min)();
                minimum = (std::numeric_limits<sample_t>::max)();
            }
        }

        double y1 = sampleToHeight(maximum);
        double y2 = sampleToHeight(minimum);

        path.lineTo(x, y1);
        path.lineTo(x, y2);
        painter.drawPath(path);
        cached_path = std::move(path);
        cached_start = window_start;
        cached_end = window_end;
        cached_width = width();
        cached_height = height();
    }
        // If we have fewer samples than pixels, we spread the samples over the screen
    else
    {
        auto line_pen = painter.pen();
        QPen point_pen;
        point_pen.setWidth(line_pen.width() * 3);
        auto pixels_per_frame = (double)this->width() / sample_count;
        double previous_x = 0;
        double previous_y = 0;
        double x = 0;

        // Add one more pixel if possible to reach the right edge of the screen
        intptr_t last;
        if (last_sample < m_data->size())
        {
            last = last_sample + 1;
        }
        else
        {
            last = last_sample;
            pixels_per_frame = (double)this->width() / (this->windowDuration() - 1);
        }

        bool display_points = (pixels_per_frame >= 10);

        for (intptr_t i = first_sample; i <= last; i++)
        {
            auto sample = m_data->sample(i);
            auto y = sampleToHeight(sample);

            if (x != 0)
            {
                QPointF p1(previous_x, previous_y);
                QPointF p2(x, y);
                if (display_points)
                {
                    painter.setPen(point_pen);
                    painter.drawPoint(p2);
                    painter.setPen(line_pen);
                }
                painter.drawLine(p1, p2);
            }
            previous_x = x;
            previous_y = y;
            x += pixels_per_frame;
        }

        // Make sure the path is invalid
        cached_path = QPainterPath();
    }
}

void Waveform::drawYAxis(QWidget *y_axis, int y1, int y2)
{
    QPainter painter(y_axis);
    auto font_metrics = painter.fontMetrics();
    auto top = QString("+%1").arg(QString::number(extrema.second, 'f', 4));
    QString center("0");
    QString bottom = QString::number(extrema.first, 'f', 4);
    int h = font_metrics.height();

    int x1 = y_axis->width() - font_metrics.width(top);
    painter.drawText(x1, y1 + h/2, top);

    int x2 = y_axis->width() - font_metrics.width(bottom);
    painter.drawText(x2, y2, bottom);

    int x3 = y_axis->width() - font_metrics.width(center);
    int y3 = y1 + (y2 - y1) / 2 + h/3 ;
    painter.drawText(x3, y3, center);
}

double Waveform::sampleToHeight(sample_t s) const
{
    const double H = (double)this->height() / 2;
    return H - s * H / magnitude; // std::numeric_limits<sample_t>::max();
}

void Waveform::informWindow()
{
    emit windowHasChanged(window_start, window_end);
}

void Waveform::setMagnitude(double value)
{
    magnitude = value;
    value = std::abs(value / (std::numeric_limits<sample_t>::min)());
    extrema = {-value, value};
}

} // namespace phonometrica