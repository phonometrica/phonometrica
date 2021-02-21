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
 * Created: 20/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/plot/sound_plot.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

SoundPlot::SoundPlot(wxWindow *parent, const Handle<Sound> &snd) :
	TimeAlignedWindow(parent), m_sound(snd), m_sel{-1, -1}
{
	m_track_mouse = Settings::get_boolean("enable_mouse_tracking");
	Bind(wxEVT_LEFT_DOWN, &SoundPlot::OnStartSelection, this);
	Bind(wxEVT_LEFT_UP, &SoundPlot::OnEndSelection, this);
	Bind(wxEVT_MOTION, &SoundPlot::OnMotion, this);
	Bind(wxEVT_LEAVE_WINDOW, &SoundPlot::OnLeaveWindow, this);
	Bind(wxEVT_MOUSEWHEEL, &SoundPlot::OnMouseWheel, this);
}

void SoundPlot::OnStartSelection(wxMouseEvent &e)
{
	// Erase the anchor (if any) and start a selection.
	auto pos = e.GetPosition();
	m_sel_start = pos.x;
	update_anchor(-1);
	e.Skip();
}

void SoundPlot::OnEndSelection(wxMouseEvent &e)
{
	auto pos = e.GetPosition();
	// Set an anchor if the mouse button was released in the same location that it was clicked.
	if (m_track_mouse && pos.x == m_sel_start)
	{
		update_anchor(pos.x);
		update_selection(PixelSelection{-1, -1});
		Refresh();
	}
	m_sel_start = -1;
	e.Skip();
}

void SoundPlot::OnMotion(wxMouseEvent &e)
{
	if (m_sel_start >= 0)
	{
		auto pos = e.GetPosition();
		double x = pos.x;

		// We don't need to refresh the plot because the signal will be sent back to us by the view
		// to which this plot is connected.
		if (x < m_sel_start) {
			update_selection(PixelSelection{x, m_sel_start});
		}
		else {
			update_selection(PixelSelection{m_sel_start, x});
		}
	}
	else if (m_track_mouse)
	{
		auto pos = ScreenToClient(wxGetMousePosition());
		update_cursor(pos.x);
	}
}

void SoundPlot::OnLeaveWindow(wxMouseEvent &e)
{
	m_sel_start = -1;
	update_cursor(-1);
	e.Skip();
}

void SoundPlot::SetSelection(PixelSelection sel)
{
	m_sel = sel;
	Refresh();
}

double SoundPlot::ClipTime(double t) const
{
    if (t < 0) {
	    t = 0;
    }
    else if (t > m_sound->duration()) {
	    t = m_sound->duration();
    }

    return t;
}

void SoundPlot::ZoomToSelection()
{
    if (HasSelection())
    {
	    auto t1 = ClipTime(XPosToTime(m_sel.from));
	    auto t2 = ClipTime(XPosToTime(m_sel.to));
	    SetTimeWindow(TimeSpan{t1, t2});
    }
}

void SoundPlot::ZoomIn()
{
	SetTimeWindow(ComputeZoomIn());
}

void SoundPlot::ZoomOut()
{
	SetTimeWindow(ComputeZoomOut());
}

void SoundPlot::ViewAll()
{
	TimeSpan win{0, m_sound->duration()};
	SetTimeWindow(win);
}

void SoundPlot::MoveForward()
{
    // Slide by 10%
    if (m_window.to < m_sound->duration())
    {
        auto delta = std::max<double>(GetWindowDuration() / 10, 0.001);
        auto t1 = ClipTime(m_window.from + delta);
        auto t2 = ClipTime(m_window.to + delta);
        SetTimeWindow(TimeSpan{t1, t2});
    }
}

void SoundPlot::MoveBackward()
{
    // Slide by 10%
    if (m_window.from >= 0)
    {
        auto delta = std::max<double>(GetWindowDuration() / 10, 0.001);
        auto t1 = ClipTime(m_window.from - delta);
        auto t2 = ClipTime(m_window.to - delta);
        SetTimeWindow(TimeSpan{t1, t2});
    }
}

void SoundPlot::EnableMouseTracking(bool value)
{
	m_track_mouse = value;

	if (!value)
	{
		m_time_anchor = -1;
		Refresh(); // erase anchor if there's one.
	}
}

void SoundPlot::OnMouseWheel(wxMouseEvent &e)
{
	if (e.GetWheelRotation() > 0) {
		update_window(ComputeZoomIn());
    }
	else {
		update_window(ComputeZoomOut());
	}
}

TimeSpan SoundPlot::ComputeZoomIn() const
{
	// Zoom in by 25% on each side
	auto zoom = GetWindowDuration() / 4;
	auto t1 = ClipTime(m_window.from + zoom);
	auto t2 = ClipTime(m_window.to - zoom);

	return TimeSpan{t1, t2};
}

TimeSpan SoundPlot::ComputeZoomOut() const
{
	// Zoom out by 50%
	auto zoom = GetWindowDuration() / 2;
	auto t1 = ClipTime(m_window.from - zoom);
	auto t2 = ClipTime(m_window.to + zoom);

	return TimeSpan{t1, t2};
}

void SoundPlot::DrawSelection(wxPaintDC &dc)
{
	if (!HasSelection()) {
		return;
	}
	auto gc = dc.GetGraphicsContext();
	if (!gc) return;
	auto height = GetHeight();
	auto path = gc->CreatePath();


	path.MoveToPoint(m_sel.from, 0.0);
	path.AddLineToPoint(m_sel.to, 0.0);
	path.AddLineToPoint(m_sel.to, height);
	path.AddLineToPoint(m_sel.from, height);
	path.AddLineToPoint(m_sel.from, 0.0);
	wxBrush brush;
	brush.SetColour(wxColour(255, 159, 41, 50));
	gc->SetBrush(brush);
	gc->FillPath(path);
}

void SoundPlot::DrawTimeAnchor(wxPaintDC &dc)
{
	if (!HasTimeAnchor()) {
		return;
	}
	dc.SetPen(wxPen(wxColour(41, 255, 159)));
	dc.DrawLine(m_time_anchor, 0.0, m_time_anchor, GetHeight());
}

void SoundPlot::DrawCursor(wxPaintDC &dc)
{
	if (m_track_mouse && HasCursor())
	{
		dc.SetPen(wxPen(wxColour(255, 0, 0, 75)));
		dc.DrawLine(m_cursor_pos, 0, m_cursor_pos, GetHeight());
	}
}

void SoundPlot::SetCursorPosition(double pos)
{
	m_cursor_pos = pos;
	Refresh();
}

void SoundPlot::SetAnchorPosition(double pos)
{
	m_time_anchor = pos;
	Refresh();
}

} // namespace phonometrica
