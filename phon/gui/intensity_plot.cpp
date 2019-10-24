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
#include <phon/speech/signal_processing.hpp>

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