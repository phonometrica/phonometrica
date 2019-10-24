/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
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

    // Borrow reference to avoid copying
    vector vec;
    vec.x = (int) input.size();
    vec.v = input.data();

	PHON_LOG("Running SWIPE");
    auto tmp = swipe(vec, sample_rate, min_pitch, max_pitch, voicing_threshold, time_step);
    pitch_data = Array<double>::from_memory(tmp.v, tmp.x);
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
