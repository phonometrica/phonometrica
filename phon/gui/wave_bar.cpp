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

#include <cmath>
#include <QMouseEvent>
#include <QDebug>
#include <phon/gui/wave_bar.hpp>
#include "wave_bar.hpp"


namespace phonometrica {

WaveBar::WaveBar(const std::shared_ptr<AudioData> &data, QWidget *parent) :
    QWidget(parent), m_data(data)
{
    setFixedHeight(50);

    auto sample_count = m_data->size();
    auto raw_data = m_data->data();
    raw_magnitude = 0;

    for (auto i = 0; i < sample_count; i++)
    {
        double m = std::abs(double(raw_data[i]));

        if (m > raw_magnitude)
            raw_magnitude = m;
    }
}

void WaveBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen;

    auto from = startPosition();
    auto to = endPosition();

    // Draw zero-crossing line
    pen.setColor(Qt::blue);
    pen.setStyle(Qt::DotLine);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawLine(0, height()/2, width(), height()/2);

    // Draw wave
    pen.setColor(Qt::black);
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    if (cached_path.isEmpty() || cached_width != width())
    {
        QPainterPath path;
        makePath(path);
        cached_path = std::move(path);
        cached_width = width();
    }
    painter.drawPath(cached_path);

    QColor col(Qt::blue);
    col = col.lighter(160);
    col.setAlpha(60);

    // If the user if making a selection backward, make sure that it is displayed properly
    auto w = int(round(std::abs(to - from)));
    auto x = int(round((from < to) ? from : to));

    if (w <= 1)
    {
        QPen pen;
        col.setAlpha(120);
        pen.setColor(col);
        painter.setPen(pen);
        painter.drawLine(x, 0, x, height());
    }
    else
    {
        painter.fillRect(x, 0, w, height(), col);
    }

    QWidget::paintEvent(event);
}

int WaveBar::sampleToYPos(double s) const
{

    const int H = this->height() / 2;
    return H - int(s * (double(H) / raw_magnitude));
}

void WaveBar::mousePressEvent(QMouseEvent *event)
{
    current_window.first = xPosToTime(event->localPos().x());
    emit selectionStarted();
}

void WaveBar::mouseReleaseEvent(QMouseEvent *event)
{
    current_window.second = xPosToTime(std::max<double>(0.0, event->localPos().x()));
    if (current_window.first == current_window.second) return;

    if (current_window.second < current_window.first) {
        std::swap(current_window.first, current_window.second);
    }
    notifySelection();
}

void WaveBar::mouseMoveEvent(QMouseEvent *event)
{
    current_window.second = xPosToTime(event->localPos().x());
    repaint();
}

void WaveBar::wheelEvent(QWheelEvent *event)
{
    QPoint degrees = event->angleDelta() / 8;
    int steps = -degrees.y() / 15;
    //steps *= 5;
    auto from = startPosition();
    auto to = endPosition();
    auto delta = to - from;
    from += steps;
    if (from < 0) {
        from = 0;
    }
    to = from + delta;
    if (to > this->width())
    {
        to = this->width();
        from = to - delta;
    }
    current_window.first = xPosToTime(from);
    current_window.second = xPosToTime(to);
    repaint();
    notifySelection();
    event->accept();
}

void WaveBar::setTimeSelection(double t1, double t2)
{
	current_window = {t1, t2};

    emit updatedXAxisSelection(startPosition(), endPosition());
    repaint();
}

double WaveBar::sampleToXPos(intptr_t s) const
{
    return s * ((double)width() / m_data->size());
}

intptr_t WaveBar::xPosToSample(double x) const
{
    return (intptr_t) round(x * m_data->size() / width()) + 1;
}

void WaveBar::notifySelection()
{
	auto from = startPosition();
	auto to = endPosition();
    emit updatedXAxisSelection(from, to);
    auto t1 = xPosToTime(from);
    auto t2 = xPosToTime(to);
    assert(t1 >= 0 && t2 <= m_data->duration());
    emit timeSelection(t1, t2);
    // FIXME: force repaint because leftward selection doesn't seem to get redrawn, somehow...
    repaint();
}

double WaveBar::timeToXPos(double t) const
{
    auto s = m_data->time_to_frame(t);
    return sampleToXPos(s);
}

double WaveBar::xPosToTime(double x) const
{
    auto s = std::min<intptr_t>(xPosToSample(x), m_data->size());
    return m_data->frame_to_time(s);
}

void WaveBar::makePath(QPainterPath &path)
{
    int x = 0;
    auto sample_count = m_data->size();
    auto raw_data = m_data->data();

    // Subtract 1 to width so that the last pixel is assigned the left-over frames.
    auto frames_per_pixel = sample_count / (this->width()-1);

    auto maximum = (std::numeric_limits<double>::min)();
    auto minimum = (std::numeric_limits<double>::max)();

    for (intptr_t i = 1; i <= sample_count; i++)
    {
        auto sample = *raw_data++;

        if (sample < minimum)
            minimum = sample;
        if (sample > maximum)
            maximum = sample;

        if (i % frames_per_pixel == 0)
        {
            int y1 = sampleToYPos(maximum);
            int y2 = sampleToYPos(minimum);

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
            maximum = (std::numeric_limits<double>::min)();
            minimum = (std::numeric_limits<double>::max)();
        }
    }

    int y1 = sampleToYPos(maximum);
    int y2 = sampleToYPos(minimum);
    path.lineTo(x, y1);
    path.lineTo(x, y2);
}

double WaveBar::magnitude() const
{
    return raw_magnitude;// / std::abs(std::numeric_limits<double>::min());
}

double WaveBar::startPosition() const
{
	return timeToXPos(current_window.first);
}

double WaveBar::endPosition() const
{
	return timeToXPos(current_window.second);
}


} // namespace phonometrica