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
 * Created: 20/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cmath>
#include <QWheelEvent>
#include <QDebug>
#include <phon/gui/speech_plot.hpp>
#include "speech_plot.hpp"


namespace phonometrica {

SpeechPlot::SpeechPlot(Runtime &rt, std::shared_ptr<AudioData> data, QWidget *parent) :
    SpeechWidget(parent), rt(rt), m_data(std::move(data))
{
    setMouseTracking(true);
}

void SpeechPlot::setWindow(double start_time, double end_time)
{
	assert(start_time >= 0 && end_time <= m_data->duration());
	moveWindow(start_time, end_time);
	repaint(); // don't update, just repaint since we are not triggering the change
}

void SpeechPlot::updateWindow(double start_time, double end_time)
{
    assert(start_time >= 0 && end_time <= m_data->duration());
	window_start = start_time;
	window_end = end_time;
    repaint();
}

void SpeechPlot::zoomIn()
{
    // Zoom in by 25% on each side
    auto zoom = windowDuration() / 4;
    auto t1 = clipTime(window_start + zoom);
    auto t2 = clipTime(window_end - zoom);
    moveWindow(t1, t2);
    repaint();
}

double SpeechPlot::clipTime(double t) const
{
    if (t < 0)
        t = 0;
    if (t > m_data->duration())
        t = m_data->duration();

    return t;
}

void SpeechPlot::zoomOut()
{
    // Zoom out by 50%
    auto zoom = windowDuration() / 2;
    auto t1 = clipTime(window_start - zoom);
    auto t2 = clipTime(window_end + zoom);
    moveWindow(t1, t2);
    repaint();
}

void SpeechPlot::zoomToSelection()
{
    if (this->hasSelection())
    {
        auto t1 = clipTime(sel_start);
        auto t2 = clipTime(sel_end);
        moveWindow(t1, t2);
        clearSelection();
        repaint();
    }
}

void SpeechPlot::viewAll()
{
    moveWindow(0.0, m_data->duration());
    repaint();
}

void SpeechPlot::moveForward()
{
    // Slide by 10%
    if (window_end < m_data->size())
    {
        auto delta = std::max<double>(windowDuration() / 10, 0.001);
        auto t1 = clipTime(window_start + delta);
        auto t2 = clipTime(window_end + delta);
        moveWindow(t1, t2);
        repaint();
    }
}

void SpeechPlot::moveBackward()
{
    // Slide by 10%
    if (window_start > 1)
    {
        auto delta = std::max<double>(windowDuration() / 10, 0.001);
        auto t1 = clipTime(window_start - delta);
        auto t2 = clipTime(window_end - delta);
        moveWindow(t1, t2);
        repaint();
    }
}

void SpeechPlot::updatePlot()
{
    // Notify the scrollbar and other plots of the change.
    emit timeSelection(window_start, window_end);
    emit windowHasChanged(window_start, window_end);
    repaint();
}

void SpeechPlot::paintEvent(QPaintEvent *event)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    if (hasSelection())
    {
        drawSelection();

        if (isMainPlot())
        {
            QPainter painter(this);
            QFontMetrics font_metric = painter.fontMetrics();
            auto x1 = timeToXPos(sel_start);
            auto x2 = timeToXPos(sel_end);
            auto label1 = QString::number(sel_start, 'f');
            auto label2 = QString::number(sel_end, 'f');
            int w1 = font_metric.width(label1);
            int w2 = font_metric.width(label2);
            int h = font_metric.height() / 2 + 1;
            if (x1 > w1) x1 -= w1;
            if (x2 + w2 > this->width()) x2 -= w2;
            painter.drawText(QPointF(x1, h), label1);
            painter.drawText(QPointF(x2, h), label2);
        }
    }

    if (hasTick())
    {
        invalidateCurrentTime();
        QPainter painter(this);
        QFontMetrics font_metric = painter.fontMetrics();
        QColor color = QColor(Qt::red).lighter(90);
        painter.setPen(color);
        auto x = timeToXPos(tick);
        painter.drawLine(QPointF(x, 0.0), QPointF(x, height()));
    }

    renderPlot(event);

	if (trackCursor())
	{
		QPainter painter(this);
		auto default_pen = painter.pen();
		QColor color = QColor(Qt::magenta).lighter(90);
		painter.setPen(color);
		auto x = timeToXPos(current_time);
		painter.drawLine(QPointF(x, 0.0), QPointF(x, height()));

		if (isMainPlot())
		{
			painter.setPen(default_pen);
			QFontMetrics font_metric = painter.fontMetrics();
			int spacing = 3;
			x += spacing;
			auto label = QString::number(current_time, 'f');
			int w = font_metric.width(label);
			int y = font_metric.height() / 2 + 2;
			if (x + w > width()) {
				x -= w + (spacing * 2);
			}
			painter.drawText(QPointF(x, y), label);
		}
	}

    QWidget::paintEvent(event);
}

void SpeechPlot::wheelEvent(QWheelEvent *event)
{
    QPoint degrees = event->angleDelta() / 8;
    int steps = -degrees.y() / 15;

    if (steps > 0)
        emit zoomInRequested(false);
    else if (steps < 0)
        emit zoomOutRequested(false);
}

void SpeechPlot::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MiddleButton)
	{
		if (hasSelection())
		{
			emit zoomToSelectionRequested(false);
		}
	}
	else
	{
		button_pressed = true;
		auto x = event->localPos().x();
		invalidateCurrentTime();
		sel_start = sel_end = xPosToTime(x);
	}
}

void SpeechPlot::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() != Qt::MiddleButton)
	{
		button_pressed = false;
		auto x = event->localPos().x();
		updateSelectionEnd(xPosToTime(x));
	}
}

void SpeechPlot::mouseMoveEvent(QMouseEvent *event)
{
    auto x = event->localPos().x();
    auto time = xPosToTime(x);

    // If no button is pressed, track the current time, otherwise set the end of the selection
    if (button_pressed)
    {
        sel_end = time;
        invalidateCurrentTime();
        emit selectionStarted();
        emit timeSelection(sel_start, sel_end);
    }
    else
    {
        current_time = (x >= 0 && x < width()) ? time : -1;
        emit currentTime(current_time, MouseTracking::Disabled);
    }

    repaint();
}

void SpeechPlot::drawSelection()
{
    // Can't display the selection.
    if (sel_end <= window_start || sel_start >= window_end) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor col(Qt::blue);
    col = col.lighter(160);
    col.setAlpha(60);

    auto start = (std::max)(window_start, sel_start);
    auto end = (std::min)(window_end, sel_end);
    auto x1 = timeToXPos(start);
    auto x2 = timeToXPos(end);
    auto w = x2 - x1;
    QRectF rect(x1, 0.0, w, height());
    painter.fillRect(rect, col);
}

void SpeechPlot::clearSelection()
{
    sel_start = -1.0;
    sel_end = -1.0;
}

void SpeechPlot::updateSelectionEnd(double t)
{
    sel_end = t;
    invalidateCurrentTime();

    if (sel_end < sel_start) {
        std::swap(sel_start, sel_end);
    }

    emit timeSelection(sel_start, sel_end);
    repaint();
}

void SpeechPlot::updateTick(double time)
{
    // If time is negative, the tick will be erased.
    tick = time;
    repaint();
}

void SpeechPlot::moveWindow(double t1, double t2)
{
    window_start = t1;
    window_end = t2;
}

std::pair<double, double> SpeechPlot::times() const
{
    if (hasSelection()) {
        return { sel_start, sel_end };
    }

    return { window_start, window_end };
}

void SpeechPlot::hideTick()
{
    updateTick(-1);
    repaint();
}

void SpeechPlot::leaveEvent(QEvent *)
{
    button_pressed = false;
    invalidateCurrentTime();
    emit currentTime(current_time, mouse_state);
    repaint();
}

void SpeechPlot::enableMouseTracking(bool enable)
{
    mouse_tracking_enabled = enable ? MouseTracking::Enabled : MouseTracking::Disabled;
}

void SpeechPlot::setCurrentTime(double time, MouseTracking tracking)
{
	mouse_state = tracking;

	if (time < 0)
	{
		// FIXME: this is not properly cleared for instants when the mouse is released
		//  (see LayerWidget::mouseReleaseEvent).
		clearCurrentTime();
	}
    else if (mouse_tracking_enabled != MouseTracking::Disabled || tracking != MouseTracking::Disabled)
    {
        current_time = time;
	    repaint();
    }
}

void SpeechPlot::setSelection(double start_time, double end_time)
{
    sel_start = start_time;
    sel_end = end_time;
    repaint();
}

void SpeechPlot::clearCurrentTime()
{
    invalidateCurrentTime();
    mouse_state = MouseTracking::Disabled;
    repaint();
}

bool SpeechPlot::needsRefresh() const
{
    return cached_end != window_end || cached_start != window_start;
}

bool SpeechPlot::trackCursor() const
{
    return hasCurrentTime() &&
	    (mouse_tracking_enabled != MouseTracking::Disabled || mouse_state != MouseTracking::Disabled);
}

void SpeechPlot::setInitialWindow()
{
    window_start = 0;
    window_end = (std::min)(DEFAULT_WINDOW_DURATION, m_data->duration());
    repaint();
}

bool SpeechPlot::hasSelection() const
{
    return sel_start >= 0 && sel_end > 0 && sel_start != sel_end && !(sel_start == window_start && sel_end == window_end);
}

} // namespace phonometrica