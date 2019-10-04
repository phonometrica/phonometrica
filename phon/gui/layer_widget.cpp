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
#include <phon/gui/layer_info_dialog.hpp>

#include <QVBoxLayout>
#include <QLabel>

namespace phonometrica {

LayerWidget::LayerWidget(const AutoAnnotation &annot, double duration, intptr_t layer_index, QWidget *parent) :
    SpeechWidget(parent), unused(annot), graph(annot->graph()), layer(graph.get(layer_index)), m_duration(duration)
{
    setMinimumHeight(50);
    setMaximumHeight(70);
    setMouseTracking(true);

	button = new QPushButton;
	setButtonIcon();
	button->setFlat(true);
	button->setCheckable(true);
	dialog = new LayerInfoDialog(this, this->layer);
	button->setToolTip(dialog->label());
}

void LayerWidget::drawYAxis(QWidget *y_axis, int y1, int y2)
{
	int side = 27;
	int y = y1 - (side / 2) + (y2 - y1) / 2;

	if (!button->parentWidget())
	{
		button->setParent(y_axis);
		button->setFixedSize(side, side);
		button->setIconSize(QSize(side-2, side-2));
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
            //assert(end_time != last_time);
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

	// Paint selected event
	// FIXME: crash when deleting an aligned event: check that the event is valid
	if (has_focus && selected_event && selected_event->valid())
	{
		if (selected_event->is_instant())
		{
			QColor col;
			col.setNamedColor("orange");
			auto old_pen = painter.pen();
			auto pen = old_pen;
			pen.setColor(col);
			auto x = timeToXPos(selected_event->start_time());
			int third = height() / 3;
			painter.setPen(pen);
			painter.drawLine(QPointF(x, 0), QPointF(x, third));
			painter.drawLine(QPointF(x, third*2), QPointF(x, height()));
			painter.setPen(old_pen);
		}
		else
		{
			auto x1 = timeToXPos(selected_event->start_time());
			auto x2 = timeToXPos(selected_event->end_time());
			QColor col;
			col.setNamedColor("orange");
			col.setAlpha(120);
			QRectF rect(x1, 0, x2-x1, height());
			painter.fillRect(rect, col);
		}
	}

    // Paint moving anchor
    if (moving_anchor_time >= 0)
    {
        auto old_pen = painter.pen();
        auto new_pen = old_pen;
        // If anchors are shared, make the width the same as a regular anchor to indicate that it will be moved too.
        int w = (sharing_anchors && hidden_anchor_time >= 0) ? 3 : 1;
        new_pen.setWidth(w);
        new_pen.setColor(Qt::green);
        pen.setStyle(Qt::DotLine);
        painter.setPen(new_pen);
        auto x = timeToXPos(moving_anchor_time);
        painter.drawLine(QPointF(x, 0), QPointF(x, width()));
        painter.setPen(old_pen);
    }

	// Ghost anchor.
	if (adding_anchor && hasGhostAnchor())
	{
		auto pen = painter.pen();
		pen.setWidth(1);
		pen.setColor(Qt::gray);
		pen.setStyle(Qt::DotLine);
		painter.setPen(pen);
		drawAnchor(painter, ghost_anchor_time, hasInstants());
	}

	// Paint temporary anchor which might get added if the user clicks on the layer.
    if (adding_anchor && hasEditAnchor())
    {
    	double anchor_time = findClosestAnchorTime(edit_anchor_time);

    	// Try to find an existing anchor that is being hovered, or a ghost anchor
    	if ((anchor_time >= 0) || anchorHasCursor(ghost_anchor_time, edit_anchor_time))
	    {
    		setCursor(Qt::PointingHandCursor);
	    }
    	// Otherwise, display a temporary anchor.
	    else
	    {
	    	setCursor(Qt::ArrowCursor);
		    auto pen = painter.pen();
		    pen.setWidth(3);
		    pen.setColor(Qt::red);
		    pen.setStyle(Qt::SolidLine);
		    painter.setPen(pen);
		    drawAnchor(painter, edit_anchor_time, hasInstants());
	    }
    }

    // Paint separator between layers.
	if (layer->index < graph.layer_count())
	{
		auto pen = painter.pen();
		pen.setWidth(2);
		pen.setColor(SpaceLine::lineColor());
		pen.setStyle(Qt::SolidLine);
		painter.setPen(pen);
		painter.drawLine(0, height(), width(), height());
	}

    QWidget::paintEvent(e);
}

void LayerWidget::setAddingAnchor(bool value)
{
	SpeechWidget::setAddingAnchor(value);
	clearEditAnchor();
}

void LayerWidget::setRemovingAnchor(bool value)
{
	SpeechWidget::setRemovingAnchor(value);
	clearEditAnchor();
}

void LayerWidget::mousePressEvent(QMouseEvent *e)
{
	auto t = timeAtCursor(e);

	if (adding_anchor)
	{
		if (hasGhostAnchor() && anchorHasCursor(ghost_anchor_time, t))
		{
			createAnchor(ghost_anchor_time, false);
		}
		else
		{
			double anchor_time = findClosestAnchorTime(t);
			// If the user clicks on an existing anchor in editing mode, create a ghost anchor on the other layers.
			if (anchor_time >= 0)
			{
				emit anchor_selected(layer->index, anchor_time);
			}
			else
			{
				// No anchor found: create a new one.
				createAnchor(t, false);
			}
		}
	}
	else if (removing_anchor)
	{
		if (removeAnchor(t, false))
		{
			clearGhostAnchor();
			clearEditAnchor();
			clearResizingEvent();
		}
	}
	else
	{
		if (sharing_anchors)
		{
			// Show lines on the other layers as soon as the mouse is pressed.
			double anchor_time = findClosestAnchorTime(t);

			if (anchor_time >= 0)
			{
				emit editing_shared_anchor(layer->index, anchor_time);
			}
		}
		dragging_anchor = true;
	}
}

void LayerWidget::mouseReleaseEvent(QMouseEvent *e)
{
    dragging_anchor = false;
	setCursor(Qt::ArrowCursor);

	if (sharing_anchors) {
    	emit editing_shared_anchor(layer->index, -1);
    }

    if (hasDroppedAnchor())
    {
        bool ok;
		double from;

        if (event_start_selected)
        {
	        from = resizing_event->start_time();
	        ok = graph.change_start_time(resizing_event, dropped_anchor_time);
        }
        else
        {
	        from = resizing_event->end_time();
            ok = graph.change_end_time(resizing_event, dropped_anchor_time);
        }

        if (ok)
        {
            event_cache.clear();
            if (sharing_anchors) {
	            emit anchor_moved(layer->index, from, dropped_anchor_time);
            }
	        emit modified();
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

    auto it = std::lower_bound(event_cache.begin(), event_cache.end(), t, EventLess());
    if (it == event_cache.end()) {
        return; // this should never happen, but we don't want to crash...
    }
    setFocus();

    auto event = *it;
    if (event->is_interval() || anchorHasCursor(event->start_time(), t))
    {
	    setSelectedEvent(event);
    }

    // Invalidate current time
	emit current_time(-1, MouseTracking::Disabled);
}

void LayerWidget::mouseMoveEvent(QMouseEvent *e)
{
	auto t = timeAtCursor(e);

	if (dragging_anchor && dragged_anchor_time >= 0)
	{
		setCursor(Qt::SizeHorCursor);
		dropped_anchor_time = t;
		emit anchor_moving(layer->index, t);
		repaint();
	}
	else if (adding_anchor)
	{
		setEditAnchor(t);
		repaint();
	}
	else
	{
		setCursor(Qt::ArrowCursor);
		trackAnchor(t);
	}

	// Force tracking in the plot if an anchor is being dragged or added.
	auto state = (dragging_anchor || adding_anchor) ? MouseTracking::Anchored : MouseTracking::Disabled;
	emit current_time(t, state);
	double time = (state == MouseTracking::Anchored) ? t : -1;
	if (sharing_anchors) {
		emit temporary_anchor(layer->index, time);
	}
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
//    else if (sharing_anchors && moving_anchor_time == time)
//    {
//    	// pass
//    }
    else if (time != hidden_anchor_time)
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

void LayerWidget::setSelectedAnchor(const AutoEvent &event, double time, bool selected)
{
    if (selected && time != 0 && time != m_duration)
    {
        setCursor(Qt::PointingHandCursor);
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
    // Allow for roughly 4 pixels before or after the exact time
    double delta = windowDuration() * 4 / width();
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
        }
        setSelectedAnchor(event, target_time, selected);
    }
}

void LayerWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    auto t = timeAtCursor(e);
	auto event = findEvent(t);
	if (event) editEvent(event);
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
                emit focus_event(index - 1, selected_event->center_time(window_start, window_end));
            }
        }
    }
    else if (e->key() == Qt::Key_Down)
    {
        if (selected_event)
        {
            auto index = layer->index;

            if (index < graph.layer_count()) {
                emit focus_event(index + 1, selected_event->center_time(window_start, window_end));
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
    if (!selected_event) return;
    
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

        if (it != event_cache.end())
        {
	        if (selected_event->is_instant())
	        {
	        	it--; assert(it != event_cache.end());
	        }
            setSelectedEvent(*it);
        }
    }
}

void LayerWidget::focusNextEvent()
{
    if (!selected_event) return;

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
        EventList::iterator it;
        if (selected_event->is_interval())
        {
	        it = std::upper_bound(event_cache.begin(), event_cache.end(), selected_event, EventLessEqual());
        }
        else
        {
            it = std::upper_bound(event_cache.begin(), event_cache.end(), selected_event, EventLess());
        }

        if (it != event_cache.end())
        {
            setSelectedEvent(*it);
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
	assert(event->valid());
    selected_event = event;
    emit event_selected(event->start_time(), event->end_time());
    repaint();
}

void LayerWidget::setEventFocus(double time)
{
	//AutoEvent event = findEvent(time);
	auto it = std::lower_bound(event_cache.begin(), event_cache.end(), time, EventLess());

	if (it != event_cache.end())
	{
		has_focus = true;
		setFocus();
		setSelectedEvent(*it);
	}
	else
	{
		unfocus();
	}
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

void LayerWidget::setAnchorSharing(bool value)
{
	sharing_anchors = !value;
}

AutoEvent LayerWidget::findEvent(double time)
{
	if (hasInstants())
	{
		auto it = std::lower_bound(event_cache.begin(), event_cache.end(), time, EventLess());

		if (it != event_cache.end() && anchorHasCursor((*it)->start_time(), time))
		{
			return *it;
		}
		else
		{
			it = std::upper_bound(event_cache.begin(), event_cache.end(), time, EventLess());

			if (it != event_cache.end() && anchorHasCursor((*it)->start_time(), time))
			{
				return *it;
			}
		}
	}
	else
	{
		auto it = std::lower_bound(event_cache.begin(), event_cache.end(), time, EventLess());
		if (it != event_cache.end()) return *it;
	}

	return nullptr;
}

double LayerWidget::timeAtCursor(QMouseEvent *e) const
{
	return xPosToTime(e->localPos().x());
}

void LayerWidget::leaveEvent(QEvent *event)
{
	clearEditAnchor();
	emit current_time(-1, MouseTracking::Disabled);

	if (sharing_anchors) {
		emit temporary_anchor(layer->index, -1);
	}
	repaint();
	QWidget::leaveEvent(event);
}

void LayerWidget::createAnchor(double time, bool silent)
{
	try
	{
		graph.add_anchor(layer->index, time, silent);
		clearEditAnchor();
		clearGhostAnchor();
		clearResizingEvent();
		updateUi();
		emit anchor_selected(layer->index, time);
		// Notify the other layers.
		if (sharing_anchors && !silent) {
			emit anchor_added(layer->index, time);
		}
	}
	catch (std::exception &e)
	{
		QMessageBox msg(QMessageBox::Critical, tr("Cannot add anchor"), e.what());
		msg.exec();
	}
}

bool LayerWidget::removeAnchor(double time, bool silent)
{
	try
	{
		auto e = findEvent(time);
		if (e && anchorHasCursor(e->end_time(), time))
		{
			bool removed = graph.remove_anchor(layer->index, e->end_time());

			if (removed)
			{
				updateUi();

				// Notify the other layers.
				if (sharing_anchors && !silent) {
					emit anchor_removed(layer->index, time);
				}

				return true;
			}
		}
	}
	catch (std::exception &e)
	{
		QMessageBox msg(QMessageBox::Critical, tr("Cannot remove anchor"), e.what());
		msg.exec();
	}

	return false;
}

void LayerWidget::updateInfo()
{
	auto text = dialog->updateInfo();
	button->setToolTip(text);
	setButtonIcon();
}

void LayerWidget::updateUi()
{
	updateEvents();
	updateInfo();
	repaint();
	emit modified();
}

void LayerWidget::rename(const String &name)
{
	layer->label = name;
	graph.set_modified(true);
	updateInfo();
}

void LayerWidget::setButtonIcon()
{
	auto path = (layer->index <= 9) ? QString(":/icons/number/%1.png").arg(layer->index) : QString(":/icons/info.png");
	button->setIcon(QIcon(path));

}

void LayerWidget::removeInfoButton()
{
	// Note: we can't put this in the destructor because the button is not owned by the widget.
	delete button;
	button = nullptr;
}

void LayerWidget::setGhostAnchorTime(double time)
{
	if (graph.anchor_exists(layer->index, time))
	{
		clearGhostAnchor();
	}
	else
	{
		ghost_anchor_time = time;
	}
	repaint();
}

double LayerWidget::findClosestAnchorTime(double time)
{
	// If the user clicks on an existing anchor in editing mode, create a ghost anchor on the other layers.
	auto it = layer->find_event(time);

	if (layer->validate(it))
	{
		auto event = *it;

		if (anchorHasCursor(event->start_time(), time))
		{
			return event->start_time();
		}
		else if (event->is_interval() && anchorHasCursor(event->end_time(), time))
		{
			return event->end_time();
		}
	}

	return -1;
}

void LayerWidget::hideAnchor(double time)
{
	auto it = layer->find_event(time);

	if (layer->validate(it) && (*it)->has_anchor(time))
	{
		hidden_anchor_time = time;
	}
	else
	{
		hidden_anchor_time = -1;
	}
}

bool LayerWidget::moveAnchor(double from, double to)
{
	bool ok = false;

	if (layer->has_instants)
	{
		auto it = layer->find_event(from);
		if (!layer->validate(it)) return false;

		if ((*it)->start_time() == from)
		{
			ok = graph.change_end_time(*it, to);
		}
	}
	else
	{
		auto it = layer->find_event(from);
		if (!layer->validate(it)) return false;

		if ((*it)->start_time() == from)
		{
			ok = graph.change_start_time(*it, to);
		}
		else if ((*it)->end_time() == from)
		{
			ok = graph.change_end_time(*it, to);
		}
	}

	if (ok)
	{
		event_cache.clear();
		emit modified();
	}

	return ok;
}

} // namespace phonometrica