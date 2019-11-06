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
 * Created: 29/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QDebug>
#include <phon/gui/pitch_plot.hpp>
#include <phon/application/settings.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/include/reset_pitch_settings_phon.hpp>
#include <phon/third_party/swipe/swipe.h>

#define MAX_PITCH_WINDOW 30

namespace phonometrica {


PitchPlot::PitchPlot(Runtime &rt, const AutoSound &sound, QWidget *parent) :
    SpeechPlot(rt, sound, parent)
{
	PHON_LOG("creating pitch plot");

	try
	{
		readSettings();
	}
	catch (std::exception &)
	{
		run_script(rt, reset_pitch_settings);
		readSettings();
	}
}

void PitchPlot::drawYAxis(QWidget *y_axis, int y1, int y2)
{
    QPainter painter(y_axis);
    auto font_metrics = painter.fontMetrics();
    auto top = QString("%1 Hz").arg(int(max_pitch));

    QString bottom = QString("%1 Hz").arg(int(min_pitch));
    int h = font_metrics.height();

    int x1 = y_axis->width() - font_metrics.width(top);
    y1 += h / 2 + 5;
    painter.drawText(x1, y1, top);

    int x2 = y_axis->width() - font_metrics.width(bottom);
    y2 -= 5;
    painter.drawText(x2, y2, bottom);
}

void PitchPlot::renderPlot(QPaintEvent *)
{
	PHON_LOG("rendering pitch plot");

	QPainter painter(this);
    auto fm = fontMetrics();

    if (windowDuration() > MAX_PITCH_WINDOW)
    {
        QString label("Zoom in to see pitch");
        auto x = width() / 2 - fm.width(label) / 2;
        auto y = height() / 2;// - fm.height() / 2;
        painter.drawText(x, y, label);
        return;
    }

    if (needsRefresh()){
        calculatePitch();
    }

    QPainterPath path;
    QPen pen(Qt::blue);
    pen.setWidth(3);
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setPen(pen);

    double t = window_start;
    bool previous = false; // no pitch before
	PHON_LOG("Painting pitch data");

    for (auto f : pitch_data)
    {
        if (std::isnan(f))
        {
            previous = false;
        }
        else if (previous)
        {
            auto x = timeToXPos(t);
            auto y = pitchToYPos(f);
            path.lineTo(QPointF(x, y));
            //painter.drawPoint(QPointF(x, y));
            previous = true;
        }
        else
        {
            auto x = timeToXPos(t);
            auto y = pitchToYPos(f);
            path.moveTo(QPointF(x, y));
            //painter.drawPoint(QPointF(x, y));
            previous = true;
        }

        t += time_step;
    }

    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawPath(path);
}

double PitchPlot::pitchToYPos(double hz) const
{
    return height() - ((hz - min_pitch) * height() / (max_pitch - min_pitch));
}

bool PitchPlot::needsRefresh() const
{
    return SpeechPlot::needsRefresh() || pitch_data.empty();
}

void PitchPlot::calculatePitch()
{
	PHON_LOG("Calculating pitch");
    auto first_sample = m_data->time_to_frame(window_start);
    auto last_sample = m_data->time_to_frame(window_end);
    auto input = m_data->get(first_sample, last_sample);
    auto sample_rate = m_data->sample_rate();

    vector vec;
    vec.x = (int) input.size();
    vec.v = input.data();

	PHON_LOG("Running SWIPE");
    auto tmp = swipe(vec, sample_rate, min_pitch, max_pitch, voicing_threshold, time_step);
    pitch_data = std::vector<double>(tmp.v, tmp.v + tmp.x);
    utils::free(tmp.v);
    cached_start = window_start;
    cached_end = window_end;
}

void PitchPlot::readSettings()
{
    String cat("pitch_tracking");
    min_pitch = Settings::get_number(rt, cat, "minimum_pitch");
    max_pitch = Settings::get_number(rt, cat, "maximum_pitch");
    time_step = Settings::get_number(rt, cat, "time_step");
    voicing_threshold = Settings::get_number(rt, cat, "voicing_threshold");
}

void PitchPlot::emptyCache()
{
	pitch_data.clear();
}

void PitchPlot::mouseMoveEvent(QMouseEvent *event)
{
	SpeechPlot::mouseMoveEvent(event);
	if (mouse_tracking == MouseTracking::Enabled)
	{
		double f = yPosToHertz(event->y());
		QString msg = QString("Frequency at cursor = %1 Hz").arg(f);
		emit statusMessage(msg);
	}
}

double PitchPlot::yPosToHertz(int y) const
{
	return (double(max_pitch - min_pitch) * (height() - y)) / height() + min_pitch;
}

} // namespace phonometrica
