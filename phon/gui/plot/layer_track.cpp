/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 27/05/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/plot/layer_track.hpp>

#ifdef __WXMSW__
#undef DrawText
#endif

namespace phonometrica {

LayerTrack::LayerTrack(wxWindow *parent, const Handle<Annotation> &annot, double duration, intptr_t layer_index) :
		SpeechWidget(parent), unused(annot), m_graph(annot->graph()), m_layer(m_graph.get(layer_index)), m_duration(duration)
{
	SetMinSize(wxSize(-1, 50));
	SetMaxSize(wxSize(-1, 70));
	Bind(wxEVT_PAINT, &LayerTrack::OnPaint, this);
	Bind(wxEVT_MOTION, &LayerTrack::OnMotion, this);
	Bind(wxEVT_LEAVE_WINDOW, &LayerTrack::OnLeaveWindow, this);
	Bind(wxEVT_LEFT_DOWN, &LayerTrack::OnLeftClick, this);
}

void LayerTrack::DrawYAxis(PaintDC &dc, const wxRect &rect)
{
	auto text = wxString::Format("%d", (int)m_layer->index);
	wxCoord w, h;
	int padding = 10;
    dc.GetTextExtent(text, &w, &h);
    int x2 = rect.width - w - padding;
    int y2 = rect.y + (rect.height - h) / 2;
    dc.DrawText(text, x2, y2);
}

void LayerTrack::OnPaint(wxPaintEvent &)
{
	bool create_controls = false;
	auto height = GetHeight();
	auto width = GetWidth();

	if (!HasValidCache())
	{
		UpdateCache();
		create_controls = true;
	}

	wxPaintDC dc(this);
	const wxColour LIGHT_YELLOW(245, 245, 215);
	const wxColour DARK_YELLOW(255, 255, 50);
    bool selected = IsSelected();

	// Set background color
	if (selected)
	{
		dc.SetBackground(LIGHT_YELLOW);
	}
	else
	{
		dc.SetBackground(*wxWHITE);
	}
	dc.Clear();
	wxPen anchor_pen(*wxBLUE, 3);
	wxPen text_pen(*wxBLACK, 1);
	double last_time = -1.0;

	for (intptr_t i = 1; i <= m_cached_events.size(); i++)
	{
		auto &event = m_cached_events[i];
		dc.SetPen(anchor_pen);
		auto start_time = event->start_time();
		auto end_time = event->end_time();
		bool is_instant = event->is_instant();

		if (start_time != last_time)
		{
			DrawAnchor(dc, start_time, is_instant);

			if (is_instant)
			{
				last_time = start_time;
			}
			else
			{
				DrawAnchor(dc, end_time, false);
				last_time = end_time;
			}
		}
		else
		{
			DrawAnchor(dc, end_time, false);
			last_time = end_time;
		}

		wxString label = event->text();
		wxStaticText *st;
		const int padding = 2;

		if (create_controls)
		{
			if (is_instant)
			{
				auto x = int(round((std::max)(0.0, TimeToXPos(start_time))));
				st = new wxStaticText(this, wxID_ANY, label, wxPoint(x-20, 0), wxSize(40, height), wxALIGN_CENTRE_HORIZONTAL);
			}
			else
			{
				auto x1 = int(round((std::max)(0.0, TimeToXPos(start_time))));
				auto x2 = int(round((std::min)(TimeToXPos(end_time), double(width))));
				st = new wxStaticText(this, wxID_ANY, label, wxPoint(x1+padding, 0), wxSize(x2-x1-padding, height), wxALIGN_CENTRE_HORIZONTAL);
				st->Bind(wxEVT_MOTION, &LayerTrack::OnCursorOverEvent, this);
				st->Bind(wxEVT_LEFT_UP, &LayerTrack::OnLeftClick, this);
			}
			m_label_ctrls.push_back(st);
		}
		else
		{
			st = m_label_ctrls[i];
			st->SetLabel(label);
		}

		// On Windows, we need to explicitly set the background color of the static texts.
		if (selected && m_selected_event->valid())
        {

            const wxColour ORANGE(247, 205, 116);

            if (m_selected_event == event)
            {
                if (m_selected_event->is_instant())
                {
                    auto old_pen = dc.GetPen();
                    dc.SetPen(wxPen(ORANGE, 3));
                    auto x = int(round(TimeToXPos(m_selected_event->start_time())));
                    int third = height / 3;
                    dc.DrawLine(wxPoint(x, 0), wxPoint(x, third));
                    dc.DrawLine(wxPoint(x, third*2), wxPoint(x, height));
                    dc.SetPen(old_pen);
                }
                else
                {
                    st->SetBackgroundColour(ORANGE);
                }
            }
            else if (event->is_interval())
            {
                st->SetBackgroundColour(LIGHT_YELLOW);
            }
        }
		else
        {
		    st->SetBackgroundColour(*wxWHITE);
        }
	}

#if 0
	// Paint selected event
	if (IsSelected() && m_selected_event->valid())
	{
		const wxColour ORANGE(247, 205, 116);
		auto old_pen = dc.GetPen();

		if (m_selected_event->is_instant())
		{
			dc.SetPen(wxPen(ORANGE, 3));
			auto x = int(round(TimeToXPos(m_selected_event->start_time())));
			int third = height / 3;
			dc.DrawLine(wxPoint(x, 0), wxPoint(x, third));
			dc.DrawLine(wxPoint(x, third*2), wxPoint(x, height));
		}
		else
		{
			dc.SetPen(wxPen(ORANGE, 1));
			auto x1 = int(round(TimeToXPos(m_selected_event->start_time())));
			auto x2 = int(round(TimeToXPos(m_selected_event->end_time())));
			auto old_brush = dc.GetBrush();
			dc.SetBrush(wxBrush(ORANGE));
			wxRect rect(x1+2, 0, x2-x1-3, height);
			dc.DrawRectangle(rect);
			dc.SetBrush(old_brush);
		}
		dc.SetPen(old_pen);
	}
#endif


#if 0
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
		pen.setColor(QColor("orange"));
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

#endif
}

void LayerTrack::DrawAnchor(wxPaintDC &dc, double time, bool is_instant)
{
	auto x = int(round(TimeToXPos(time)));
	int height = GetHeight();

	if (is_instant)
	{
		int third = height / 3;
		dc.DrawLine(wxPoint(x, 0), wxPoint(x, third));
		dc.DrawLine(wxPoint(x, third*2), wxPoint(x, height));
	}
	else
	{
		dc.DrawLine(wxPoint(x, 0), wxPoint(x, height));
	}

#if 0
    // If an anchor is being dragged, don't paint the anchor but paint its drop target.
    if (hasDroppedAnchor() && dragged_anchor_time == time)
    {
        auto x = TimeToXPos(dropped_anchor_time);
        auto pen = painter.pen();
        QPen anchor_pen;
        anchor_pen.setColor(Qt::green);
        anchor_pen.setWidth(3);
        painter.setPen(anchor_pen);
        painter.drawLine(QPointF(x, 0), QPointF(x, height()));
        painter.setPen(pen);
    }
    else if (time != hidden_anchor_time)
    {
        auto x = TimeToXPos(time);

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
#endif
}

void LayerTrack::OnMotion(wxMouseEvent &e)
{
	auto msg = wxString::Format("Layer %d", (int)m_layer->index);
	wxString title = m_layer->label;
	if (!title.IsEmpty())
	{
		msg.Append(" (");
		msg.Append(title);
		msg.Append(")");
	}
	msg.Append(": ");
	if (m_layer->has_instants) {
		msg.Append("instant");
	}
	else {
		msg.Append("span");
	}
	auto t = XPosToTime(e.GetPosition().x);
	auto index = (int) m_graph.time_to_event_index(m_layer->index, t);
	msg.Append(wxString::Format(" %d/%d", index, (int)m_layer->count()));

	update_status(msg);
}

void LayerTrack::OnCursorOverEvent(wxMouseEvent &e)
{
	auto st = dynamic_cast<wxStaticText*>(e.GetEventObject());

	if (likely(st != nullptr))
	{
		// Convert position from wxStaticText coordinates to LayerTrack coordinates.
		auto pos = st->ClientToScreen(e.GetPosition());
		pos = this->ScreenToClient(pos);
		// Forward the event to the layer track.
		e.Skip(false);
		e.SetPosition(pos);
		OnMotion(e);
	}
}

void LayerTrack::OnLeaveWindow(wxMouseEvent &e)
{
	update_status(wxString());
}

void LayerTrack::UpdateCache()
{
	for (auto st : m_label_ctrls)
	{
		st->Unbind(wxEVT_MOTION, &LayerTrack::OnCursorOverEvent, this);
		st->Unbind(wxEVT_LEFT_UP, &LayerTrack::OnLeftClick, this);
		st->Destroy();
	}
	m_label_ctrls.clear();
	m_cached_events = FilterEvents(m_window.first, m_window.second);
	m_cached_size = GetSize();
}

void LayerTrack::SetSelectedEvent(const AutoEvent &e)
{
	m_selected_event = e;
}

void LayerTrack::OnLeftClick(wxMouseEvent &e)
{
	auto pos = this->ScreenToClient(wxGetMousePosition());
	m_selected_event = XPosToEvent(pos.x);
	update_selected_event(m_layer->index, m_selected_event);
}

AutoEvent LayerTrack::XPosToEvent(int x) const
{
	auto t = XPosToTime(x);
	return m_graph.time_to_event(m_layer->index, t);
}

void LayerTrack::ClearSelectedEvent()
{
	SetSelectedEvent(nullptr);
}

int LayerTrack::GetIndex() const
{
	return m_layer->index;
}


} // namespace phonometrica
