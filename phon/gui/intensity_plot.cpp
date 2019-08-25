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
 * Created: 31/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <QDebug>
#include <phon/gui/intensity_plot.hpp>
#include <phon/speech/signal_processing.hpp>

namespace phonometrica {

IntensityPlot::IntensityPlot(Runtime &rt, std::shared_ptr<AudioData> data, QWidget *parent) :
    SpeechPlot(rt, std::move(data), parent)
{

}

void IntensityPlot::renderPlot(QPaintEvent *)
{
    QPainter painter(this);
    QPainterPath path;
    auto fm = fontMetrics();
    auto pen = painter.pen();
    painter.setRenderHint(QPainter::Antialiasing,true);
    pen.setColor(Qt::green);
    painter.setPen(pen);

    if (needsRefresh()) {
        calculateIntensity();
    }

    double t = window_start + time_step;// / 2;
    bool previous = false; // no pitch before
    int i = 0;

    for (auto dB : db_data)
    {
        if (!std::isfinite(dB))
        {
            previous = false;
        }
        else if (previous)
        {
            auto x = timeToXPos(t);
            auto y = intensityToYPos(dB);
            path.lineTo(QPointF(x, y));
            //qDebug() << "t =" << t << " dB =" << dB;
            previous = true;
        }
        else
        {
            auto x = timeToXPos(t);
            auto y = intensityToYPos(dB);
            //qDebug() << "t =" << t << " dB =" << dB;
            path.moveTo(QPointF(x, y));
            previous = true;
        }

        t += time_step;
    }

    if (trackCursor())
    {
        auto x = timeToXPos(current_time) + 5;
        auto f = findValueAtTime(current_time);

        if (std::isfinite(f))
        {
            auto y = fm.height() / 2 + 2;
            auto label = QString("%1 dB").arg(f);
            painter.drawText(QPointF(x, y), label);
        }
    }

    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawPath(path);
}

void IntensityPlot::drawYAxis(QWidget *y_axis, int y1, int y2)
{
    QPainter painter(y_axis);
    auto font_metrics = painter.fontMetrics();
    auto top = QString("%1 dB").arg(int(max_dB));

    QString bottom = QString("%1 dB").arg(int(min_dB));
    int h = font_metrics.height();

    int x1 = y_axis->width() - font_metrics.width(top);
    y1 += h / 2 + 5;
    painter.drawText(x1, y1, top);

    int x2 = y_axis->width() - font_metrics.width(bottom);
    y2 -= 5;
    painter.drawText(x2, y2, bottom);
}

double IntensityPlot::intensityToYPos(double dB) const
{
    return height() - ((dB - min_dB) * height() / (max_dB - min_dB));
}

bool IntensityPlot::needsRefresh() const
{
    return SpeechPlot::needsRefresh() || db_data.empty();
}

void IntensityPlot::calculateIntensity()
{
    auto start_pos = m_data->time_to_frame(window_start);
    auto end_pos = m_data->time_to_frame(window_end);
    auto input = m_data->real_data(start_pos, end_pos);

    // Praat's settings
    double min_pitch = 100;
    double effective_duration = 3.2 / min_pitch;
    int window_size = int(std::ceil(effective_duration * m_data->sample_rate()));

    db_data = speech::get_intensity(input, m_data->sample_rate(), window_size, time_step);
    cached_start = window_start;
    cached_end = window_end;
}

double IntensityPlot::findValueAtTime(double time)
{
    double current_time = window_start + time_step; // / 2;
    double previous_value = 0;
    double previous_time = 0;

    for (auto f : db_data)
    {
        if (current_time == time) {
            return f;
        }
        else if (time > previous_time && time < current_time)
        {
            if (!std::isfinite(previous_value) || !std::isfinite(f)) {
                return std::nan("");
            }
            // Simple linear interpolation
            auto diff = time - previous_time;
            auto delta = f - previous_value;

            return previous_value + (diff * delta / time_step);
        }
        previous_value = f;
        previous_time = current_time;
        current_time += time_step;
    }

    return std::nan("");
}
} // namespace phonometrica