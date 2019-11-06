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
 * Created: 31/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QDebug>
#include <QMessageBox>
#include <phon/gui/intensity_plot.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/application/settings.hpp>
#include <phon/include/reset_intensity_settings_phon.hpp>
#include <phon/analysis/signal_processing.hpp>

namespace phonometrica {

IntensityPlot::IntensityPlot(Runtime &rt, const AutoSound &sound, QWidget *parent) :
    SpeechPlot(rt, sound, parent)
{
	PHON_LOG("creating intensity plot");

	try
	{
		readSettings();
	}
	catch (std::exception &)
	{
		run_script(rt, reset_intensity_settings);
		readSettings();
	}
}

void IntensityPlot::renderPlot(QPaintEvent *)
{
	PHON_LOG("rendering intensity plot");

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
	db_data = m_sound->get_intensity(start_pos, end_pos, time_step);
    cached_start = window_start;
    cached_end = window_end;
}

void IntensityPlot::readSettings()
{
	String cat("intensity");
	min_dB = Settings::get_number(rt, cat, "minimum_intensity");
	max_dB = Settings::get_number(rt, cat, "maximum_intensity");
	time_step = Settings::get_number(rt, cat, "time_step");
}

void IntensityPlot::emptyCache()
{
	db_data.clear();
}

double IntensityPlot::yPosToIntensity(int y) const
{
	return (double(max_dB - min_dB) * (height() - y)) / height() + min_dB;
}

void IntensityPlot::mouseMoveEvent(QMouseEvent *event)
{
	SpeechPlot::mouseMoveEvent(event);
	if (mouse_tracking == MouseTracking::Enabled)
	{
		double dB = yPosToIntensity(event->y());
		QString msg = QString("Intensity at cursor = %1 dB").arg(dB);
		emit statusMessage(msg);
	}
}

} // namespace phonometrica