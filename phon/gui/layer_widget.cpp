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
 * Created: 24/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cmath>
#include <QDebug>
#include <QPushButton>
#include <QDialog>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QKeyEvent>
#include <phon/gui/layer_widget.hpp>
#include <phon/gui/popup_text_editor.hpp>
#include <phon/gui/space_line.hpp>

#include <QVBoxLayout>
#include <QLabel>

namespace phonometrica {

LayerWidget::LayerWidget(AGraph &graph, double duration, intptr_t layer_index, QWidget *parent) :
    SpeechWidget(parent), graph(graph), layer(graph.get(layer_index)), m_duration(duration)
{
    setMinimumHeight(50);
    setMaximumHeight(70);
    setMouseTracking(true);

	button = new QPushButton;
	button->setIcon(QIcon(":/icons/info.png"));
	button->setFlat(true);
	button->setCheckable(true);

	dialog = new QDialog(this, Qt::Window|Qt::FramelessWindowHint);
	auto layer = graph.get(layer_index).get();
	QString label = QString("<b>Layer %1</b>").arg(layer_index);
	if (!layer->label.empty()) label.append(QString(": %1").arg(layer->label));
	label.append("<br/><b>Event type</b>: ");
	if (layer->has_instants) {
		label.append("instants");
	}
	else {
		label.append("intervals");
	}
	button->setToolTip(label);

	auto w = new QLabel(label);
	auto vl = new QVBoxLayout;
	vl->addWidget(w);
	dialog->setLayout(vl);
}

void LayerWidget::drawYAxis(QWidget *y_axis, int y1, int y2)
{
	int side = 20;
	int y = y1 - (side / 2) + (y2 - y1) / 2;

	if (!button->parentWidget())
	{
		button->setParent(y_axis);
		button->setFixedSize(side, side);
	}
	button->move(20, y);
	button->show();
	int y3 = y - dialog->size().height() - 10;
	auto pt = y_axis->mapToGlobal(QPoint(30, y3)); // 30 is half the fixed size of the axis
	int x = pt.x() - (dialog->size().width() / 2);
	pt.setX(x);
	dialog->move(pt);

	connect(button, &QPushButton::clicked, [this](bool checked) {
		dialog->setVisible(checked);
	});
}

void LayerWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    // Set background color
    QPalette pal = palette();
    if (has_focus)
    {
        auto col = QColor(Qt::yellow);
        col.setAlpha(20);
        pal.setColor(QPalette::Background, col);
    }
    else
    {
        pal.setColor(QPalette::Background, Qt::white);
    }
    setAutoFillBackground(true);
    setPalette(pal);

    // Paint selected event
    if (has_focus && selected_event && selected_event->is_interval())
    {
        auto x1 = timeToXPos(selected_event->start_time());
        auto x2 = timeToXPos(selected_event->end_time());
        QColor col; // = QColor(Qt::yellow);
        col.setNamedColor("orange");
        col.setAlpha(120);
        QRectF rect(x1, 0, x2-x1, height());
        painter.fillRect(rect, col);
    }

    auto pen = painter.pen();
    auto anchor_color = QColor(Qt::blue).darker(130);

    // Paint events
    if (needsRefresh())
    {
        updateEvents();
    }
    double last_time = -1.0;

    for (auto &event : event_cache)
    {
        pen.setWidth(3);
        pen.setColor(anchor_color);
        painter.setPen(pen);

        auto start_time = event->start_time();
        auto end_time = event->end_time();
        bool is_instant = event->is_instant();


	    if (start_time != last_time)
        {
	        drawAnchor(painter, start_time, is_instant);

            if (is_instant)
            {
                last_time = start_time;
            }
            else
            {
	            drawAnchor(painter, end_time, false);
                last_time = end_time;
            }
        }
        else
        {
            assert(end_time != last_time);
	        drawAnchor(painter, end_time, false);
            last_time = end_time;
        }

        QString label(event->text());
        QRectF rect;

        if (is_instant)
        {
            auto x = std::max(0.0, timeToXPos(start_time));
            rect = QRectF(x-20, 0, 40, height());
        }
        else
        {
            auto x1 = std::max(0.0, timeToXPos(start_time));
            auto x2 = std::min(timeToXPos(end_time), double(width()));
            rect = QRectF(x1+2, 0, x2-x1-2, height());
        }

        pen.setColor(Qt::black);
        painter.setPen(pen);
        auto flags = Qt::AlignVCenter|Qt::AlignHCenter|Qt::TextWordWrap;
        painter.drawText(rect, flags, label);
    }

    // Paint moving anchor
    if (moving_anchor_time >= 0)
    {
        auto old_pen = painter.pen();
        auto new_pen = old_pen;
        new_pen.setWidth(1);
        new_pen.setColor(Qt::green);
        pen.setStyle(Qt::DotLine);
        painter.setPen(new_pen);
        auto x = timeToXPos(moving_anchor_time);
        painter.drawLine(QPointF(x, 0), QPointF(x, width()));
        painter.setPen(old_pen);
    }

    if (layer->index < graph.layer_count())
    {
        auto pen = painter.pen();
        pen.setWidth(2);
        pen.setColor(SpaceLine::lineColor());
        painter.setPen(pen);
        painter.drawLine(0, height(), width(), height());
    }

    QWidget::paintEvent(e);
}

void LayerWidget::mousePressEvent(QMouseEvent *event)
{
    dragging_anchor = true;

    // need to set resizing event here
//    auto t = xPosToTime(event->localPos().x());
//    dropped_anchor_time = t;
//    emit anchor_moving(layer_index, t);
//    repaint();
}

void LayerWidget::mouseReleaseEvent(QMouseEvent *e)
{
    dragging_anchor = false;

    if (hasDroppedAnchor())
    {
        bool ok;

        if (event_start_selected)
        {
            ok = graph.change_start_time(resizing_event, dropped_anchor_time);
        }
        else
        {
            ok = graph.change_end_time(resizing_event, dropped_anchor_time);
        }

        if (ok)
        {
            emit modified();
            event_cache.clear();
        }
        else
        {
            QMessageBox box(QMessageBox::Critical, "Error", "Cannot move anchor");
            box.exec();
        }
        clearResizingEvent();
        emit anchor_has_moved(layer->index);
        repaint();
    }

    if (e->modifiers() == Qt::NoModifier)
    {
        has_focus = true;
        emit got_focus(layer->index);
    }
    auto t = xPosToTime(e->localPos().x());
    // Set selection if we clicked an interval
    if (this->has_instants()) {
        return;
    }

    auto it = std::lower_bound(event_cache.begin(), event_cache.end(), t, EventLess());
    if (it == event_cache.end()) {
        return; // this should never happen, but we don't want to crash...
    }
    setFocus();
    setSelectedEvent(*it);
}

void LayerWidget::setWindow(double start_time, double end_time)
{
    window_start = start_time;
    window_end = end_time;
    repaint();
}

void LayerWidget::drawAnchor(QPainter &painter, double time, bool is_instant)
{
    // If an anchor is being dragged, don't paint the anchor but paint its drop target.
    if (hasDroppedAnchor() && dragged_anchor_time == time)
    {
        auto x = timeToXPos(dropped_anchor_time);
        auto pen = painter.pen();
        QPen anchor_pen;
        anchor_pen.setColor(Qt::green);
        anchor_pen.setWidth(3);
        painter.setPen(anchor_pen);
        painter.drawLine(QPointF(x, 0), QPointF(x, height()));
        painter.setPen(pen);
    }
    else
    {
        auto x = timeToXPos(time);

        if (is_instant)
        {
        	int third = height() / 3;
	        painter.drawLine(QPointF(x, 0), QPointF(x, third));
	        painter.drawLine(QPointF(x, third*2), QPointF(x, height()));
        }
        else
        {
	        painter.drawLine(QPointF(x, 0), QPointF(x, height()));
        }
    }
}

void LayerWidget::mouseMoveEvent(QMouseEvent *e)
{
    auto t = xPosToTime(e->localPos().x());

    if (dragging_anchor && dragged_anchor_time >= 0)
    {
        dropped_anchor_time = t;
        emit anchor_moving(layer->index, t);
        repaint();
    }
    else
    {
        trackAnchor(t);
    }

    // Don't force tracking in the plots.
    emit current_time(t, false);
}

void LayerWidget::setSelectedAnchor(const AutoEvent &event, double time, bool selected)
{
    if (selected && time != 0 && time != m_duration)
    {
        setCursor(Qt::SizeHorCursor);
        dragged_anchor_time = time;
        // The event might not actually be resized if the anchor is not clicked, but we store it here
        // to avoid recalculating it when/if the user clicks on the anchor.
        resizing_event = event;
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        dragged_anchor_time = -1;
        resizing_event.reset();
    }
}

bool LayerWidget::needsRefresh() const
{
    return cached_end != window_end || cached_start != window_start || event_cache.empty();
}

bool LayerWidget::anchorHasCursor(double anchor_time, double current_time) const
{
    // Allow for roughly 3 pixels before or after the exact time
    double delta = windowDuration() * 3 / width();
    return (current_time >= anchor_time - delta) && (current_time <= anchor_time + delta);
}

bool LayerWidget::eventHasCursor(const Event &event, double time, double *out_time)
{
    if (anchorHasCursor(event.end_time(), time))
    {
        *out_time = event.end_time();
        event_start_selected = false;
        return true;
    }
    if (event.is_interval() && anchorHasCursor(event.start_time(), time))
    {
        *out_time = event.start_time();
        event_start_selected = true;
        return true;
    }
    *out_time = -1;
    return false;
}

void LayerWidget::trackAnchor(double t)
{
    auto lower = std::lower_bound(event_cache.begin(), event_cache.end(), t, EventLess());

    if (lower == event_cache.end()) {
        return;
    }

    auto &event = *lower;
    double target_time;

    if (eventHasCursor(*event, t, &target_time)) {
        setSelectedAnchor(event, target_time);
    }
    else
    {
        auto greater = std::lower_bound(event_cache.begin(), event_cache.end(), t, EventLess());
        bool selected = false;

        if (greater != event_cache.end())
        {
            selected = eventHasCursor(**greater, t, &target_time);
            if (selected) qDebug() << "selected from the right";
        }
        setSelectedAnchor(event, target_time, selected);
    }
}

void LayerWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	double x = e->localPos().x();
    auto t = xPosToTime(x);

    if (this->has_intervals())
    {
        auto it = std::lower_bound(event_cache.begin(), event_cache.end(), t, EventLess());
        if (it != event_cache.end()) editEvent(*it);
    }
    else
    {
    	auto it = std::lower_bound(event_cache.begin(), event_cache.end(), t, EventLess());

    	if (it != event_cache.end() && matchInstantTime((*it)->start_time(), x))
    	{
    		editEvent(*it);
    	}
    	else
	    {
    		it = std::upper_bound(event_cache.begin(), event_cache.end(), t, EventLess());

    		if (it != event_cache.end() && matchInstantTime((*it)->start_time(), x))
		    {
    			editEvent(*it);
		    }
	    }
    }
}

void LayerWidget::unfocus()
{
    has_focus = false;
    selected_event.reset();
    repaint();
}

void LayerWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Left)
    {
        focusPreviousEvent();
    }
    else if (e->key() == Qt::Key_Right)
    {
        focusNextEvent();
    }
    else if (e->key() == Qt::Key_Up)
    {
        if (selected_event)
        {
            auto index = layer->index;

            if (index > 1) {
                emit focus_event(index - 1, selected_event->center_time());
            }
        }
    }
    else if (e->key() == Qt::Key_Down)
    {
        if (selected_event)
        {
            auto index = layer->index;

            if (index < graph.layer_count()) {
                emit focus_event(index + 1, selected_event->center_time());
            }
        }
    }
    else if (e->key() == Qt::Key_Return ||  e->key() == Qt::Key_Enter)
    {
        if (selected_event) {
            editEvent(selected_event);
        }
    }

    QWidget::keyPressEvent(e);
}

void LayerWidget::focusPreviousEvent()
{
    assert(selected_event);
    
    if (selected_event == *event_cache.begin())
    {
        auto previous_event = graph.previous_event(layer->index, selected_event);
        if (!previous_event) return;
        auto duration = windowDuration();
        auto delta = duration / 20; // 5%
        auto new_end = clipRight(previous_event->end_time() + delta);
        auto new_start = clipLeft(new_end - duration);
        window_start = new_start;
        window_end = new_end;
        updateEvents();
        setSelectedEvent(previous_event);
        emit window_moved(new_start, new_end);
    }
    else
    {
        auto it = std::lower_bound(event_cache.begin(), event_cache.end(), selected_event, EventLess());

        if (it >= event_cache.begin())
        {
            setSelectedEvent(*it);
        }
    }
}

void LayerWidget::focusNextEvent()
{
    assert(selected_event);

    if (selected_event->is_interval())
    {
        if (selected_event == event_cache.last())
        {
            auto next_event = graph.next_event(layer->index, selected_event);
            if (!next_event) return;
            auto duration = windowDuration();
            auto delta = duration / 20; // 5%
            auto new_start = clipLeft(next_event->start_time() - delta);
            auto new_end = clipRight(new_start + duration);
            window_start = new_start;
            window_end = new_end;
            updateEvents();
            setSelectedEvent(next_event);
            emit window_moved(new_start, new_end);
        }
        else
        {
            auto it = std::upper_bound(event_cache.begin(), event_cache.end(), selected_event, IntervalLessEqual());

            if (it != event_cache.end())
            {
                setSelectedEvent(*it);
            }
        }
    }
}

void LayerWidget::updateEvents()
{
    event_cache = filter_events(window_start, window_end);
    cached_start = window_start;
    cached_end = window_end;
}

void LayerWidget::setSelectedEvent(const AutoEvent &event)
{
    selected_event = event;
    emit interval_selected(event->start_time(), event->end_time());
    repaint();
}

void LayerWidget::setEventFocus(double time)
{
    auto it = std::lower_bound(event_cache.begin(), event_cache.end(), time, EventLess());

    if (it == event_cache.end())
    {
        unfocus(); // shouldn't happen
        return;
    }
    has_focus = true;
    setFocus();
    setSelectedEvent(*it);
}

void LayerWidget::editEvent(AutoEvent &event)
{
	// Find center of the interval
	auto x1 = timeToXPos(event->start_time());
	auto x2 = timeToXPos(event->end_time());
	int x = int(x1 + (x2-x1)/2);
	QPoint pos(x, 0);
	pos = mapToGlobal(pos);
	pos.setY(pos.y() + edit_y_shift);
	QString text = event->text();

	PopupTextEditor editor(text, pos, this);

    if (editor.exec() == QDialog::Accepted)
    {
        auto new_text = editor.text();
        if (new_text != text)
        {
            graph.set_event_text(event, new_text);
            emit modified();
        }
    }
    int shift = editor.yshift();
    if (shift != 0) edit_y_shift = shift;
}

void LayerWidget::clearResizingEvent()
{
    resizing_event.reset();
    dragged_anchor_time = -1;
    dropped_anchor_time = -1;
}

void LayerWidget::followMovingAnchor(double time)
{
    moving_anchor_time = time;
    repaint();
}

bool LayerWidget::matchInstantTime(double time, double xpos) const
{
	double pos = timeToXPos(time);
	return std::abs(pos - xpos) <= 5;
}

void LayerWidget::setAnchorSharing(bool value)
{
	sharing_anchors = !value;
}

} // namespace phonometrica