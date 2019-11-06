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

#include <QDebug>
#include <QMessageBox>
#include <phon/runtime/runtime.hpp>
#include <phon/application/settings.hpp>
#include <phon/gui/waveform.hpp>
#include <phon/include/reset_waveform_settings_phon.hpp>

namespace phonometrica {

Waveform::Waveform(Runtime &rt, const AutoSound &sound, QWidget *parent) :
    SpeechPlot(rt, sound, parent)
{
	PHON_LOG("creating waveform");

	try
	{
		readSettings();
	}
	catch (std::exception &)
	{
		run_script(rt, reset_waveform_settings);
		readSettings();
	}
}

void Waveform::renderPlot(QPaintEvent *)
{
	PHON_LOG("rendering waveform");

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
	// If the number of samples to display is greater than the number of pixels,
    // map several frames to one pixel. We find the maximum and minimum amplitudes
    // over the range of frames, and we draw a line from the previous minimum to the
    // current maximum, and from the current maximum to the current mimimum.

    auto first_sample = m_data->time_to_frame(window_start);
    auto last_sample = m_data->time_to_frame(window_end);
    auto sample_count = last_sample - first_sample + 1;
    auto raw_data = m_data->data() + first_sample;

    if (scaling == Scaling::Global)
    {
    	setMagnitude(global_magnitude);
    }
    else if (scaling == Scaling::Local)
    {
	    setLocalMagnitude(raw_data, raw_data + sample_count);
    }

    // Don't create the painter until we are done sending events to the Y axis, otherwise it will crash.
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	QPen pen;
	pen.setColor(Qt::black);
	painter.setPen(pen);


    if (sample_count > this->width())
    {
        QPainterPath path;
        int x = 0;
        assert(window_end <= m_data->duration());

        // Subtract 1 to width so that the last pixel is assigned the left-over frames.
        auto frames_per_pixel = sample_count / (this->width()-1);

        auto maximum = -(std::numeric_limits<double>::max)();
        auto minimum = (std::numeric_limits<double>::max)();
        intptr_t min_index = (std::numeric_limits<intptr_t>::min)();
	    intptr_t max_index = (std::numeric_limits<intptr_t>::min)();

        for (intptr_t i = 1; i <= sample_count; i++)
        {
            auto sample = *raw_data++;

            if (sample < minimum)
            {
                minimum = sample;
                min_index = i;
            }
            if (sample > maximum)
            {
                maximum = sample;
                max_index = i;
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
                maximum = -(std::numeric_limits<double>::max)();
                minimum = (std::numeric_limits<double>::max)();
                min_index = max_index = (std::numeric_limits<intptr_t>::min)();
            }
        }

        double y1 = sampleToHeight(maximum);
        double y2 = sampleToHeight(minimum);
        if (min_index < max_index) {
        	std::swap(y1, y2);
        }

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
    painter.drawText(x2, y2 - 2, bottom);

    int x3 = y_axis->width() - font_metrics.width(center);
    int y3 = y1 + (y2 - y1) / 2 + h/3 ;
    painter.drawText(x3, y3, center);
}

double Waveform::sampleToHeight(double s) const
{
    const double H = (double)this->height() / 2;
    return H - s * H / magnitude;
}

void Waveform::informWindow()
{
    emit windowHasChanged(window_start, window_end);
}

void Waveform::setMagnitude(double value)
{
    magnitude = value;
    extrema = {-value, value};
}

void Waveform::updateWindow(double start_time, double end_time)
{
	SpeechPlot::updateWindow(start_time, end_time);
	informWindow();
}

void Waveform::moveWindow(double t1, double t2)
{
	SpeechPlot::moveWindow(t1, t2);
	emit windowHasChanged(t1, t2);
}

void Waveform::readSettings()
{
    String cat("waveform");
	auto m = Settings::get_number(rt, cat, "magnitude");

	if (m <= 0 || m > 1.0)
	{
		QMessageBox::warning(this, tr("Invalid waveform settings"), tr("Your waveform settings have an invalid magnitude and will be reinitialized."));
    	throw std::runtime_error("");
	}
	setMagnitude(m);

    String method = Settings::get_string(rt, cat, "scaling");

    if (method == "global")
    {
	    scaling = Scaling::Global;
    }
    else if (method == "local")
    {
	    scaling = Scaling::Local;
    }
    else if (method == "fixed")
    {
	    scaling = Scaling::Fixed;
    }
    else
    {
    	QMessageBox::warning(this, tr("Invalid waveform settings"), tr("Your waveform settings are invalid and will be reinitialized."));
    	throw std::runtime_error("");
	}
}

void Waveform::emptyCache()
{
	cached_path = QPainterPath();
}

void Waveform::setLocalMagnitude(const double *from, const double *to)
{
	double e1 = std::abs(double(*std::max_element(from, to)));
	double e2 = std::abs(double(*std::min_element(from, to)));
	auto m = (std::max)(e1, e2);
	setMagnitude(m);
}

void Waveform::setGlobalMagnitude(double value)
{
	global_magnitude = value;
}

void Waveform::mouseMoveEvent(QMouseEvent *event)
{
	SpeechPlot::mouseMoveEvent(event);
	if (mouse_tracking == MouseTracking::Enabled)
	{
		double amp = yPosToAmplitude(event->y());
		QString msg = QString("Amplitude at cursor = %1").arg(amp);
		emit statusMessage(msg);
	}
}

double Waveform::yPosToAmplitude(int y) const
{
	const double H = (double)this->height() / 2;
	return (H - y) * magnitude / H;
}

} // namespace phonometrica