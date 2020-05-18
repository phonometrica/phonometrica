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
 * Created: 20/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cmath>
#include <QWheelEvent>
#include <QDebug>
#include <phon/gui/speech_plot.hpp>

namespace phonometrica {

SpeechPlot::SpeechPlot(Runtime &rt, const AutoSound &sound, QWidget *parent) :
    SpeechWidget(parent), rt(rt), m_sound(sound), m_data(sound->data())
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
	renderPlot(event);

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

    if (hasPersistentCursor())
    {
	    QPainter painter(this);
        QFontMetrics font_metric = painter.fontMetrics();
        auto pen = painter.pen();
        pen.setColor(QColor("orange"));
        painter.setPen(pen);
        auto x = timeToXPos(m_persistent_cursor);
        painter.drawLine(QPointF(x, 0.0), QPointF(x, height()));
    }

    if (hasTick())
    {
	    QPainter painter(this);
        invalidateCurrentTime();
        QFontMetrics font_metric = painter.fontMetrics();
        QColor color = QColor(Qt::red).lighter(90);
        painter.setPen(color);
        auto x = timeToXPos(tick);
        painter.drawLine(QPointF(x, 0.0), QPointF(x, height()));
    }

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
	if (event->button() == Qt::RightButton && mouse_tracking == MouseTracking::Enabled)
	{
		emit persistentCursorRequested(-1);
	}
	else if (event->button() == Qt::MiddleButton)
	{
		if (hasSelection())
		{
			emit zoomToSelectionRequested(false);
		}
	}
	else if (event->button() == Qt::LeftButton)
	{
		button_pressed = true;
		auto x = event->localPos().x();
		invalidateCurrentTime();
		sel_start = sel_end = xPosToTime(x);
	}
}

void SpeechPlot::mouseReleaseEvent(QMouseEvent *event)
{
	auto x = event->localPos().x();
	auto t = xPosToTime(x);

	if (event->button() == Qt::LeftButton)
	{
		button_pressed = false;

		if (t == sel_start && t == sel_end && mouse_tracking == MouseTracking::Enabled)
		{
			emit persistentCursorRequested(t);
		}
		updateSelectionEnd(t);
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
    emit statusMessage(QString("Duration of selection: %1 s").arg(selectionDuration()));
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
	mouse_tracking = enable ? MouseTracking::Enabled : MouseTracking::Disabled;
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
    else if (mouse_tracking != MouseTracking::Disabled || tracking != MouseTracking::Disabled)
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
	    (mouse_tracking != MouseTracking::Disabled || mouse_state != MouseTracking::Disabled);
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

void SpeechPlot::updateSettings()
{
	readSettings();
	emptyCache();
	repaint();
	emit yAxisModified();
}

void SpeechPlot::setPersistentCursor(double value)
{
	clearCurrentTime();
	m_persistent_cursor = value;
}

} // namespace phonometrica