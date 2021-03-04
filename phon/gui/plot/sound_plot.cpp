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

#ifdef __WXMSW__
#undef DrawText
#endif

namespace phonometrica {

SoundPlot::SoundPlot(wxWindow *parent, const Handle<Sound> &snd) :
	TimeAlignedWindow(parent), m_sound(snd)
{
	m_track_mouse = Settings::get_boolean("enable_mouse_tracking");
	Bind(wxEVT_LEFT_DOWN, &SoundPlot::OnStartSelection, this);
	Bind(wxEVT_LEFT_UP, &SoundPlot::OnEndSelection, this);
	Bind(wxEVT_MOTION, &SoundPlot::OnMotion, this);
	Bind(wxEVT_LEAVE_WINDOW, &SoundPlot::OnLeaveWindow, this);
	Bind(wxEVT_MOUSEWHEEL, &SoundPlot::OnMouseWheel, this);
	Bind(wxEVT_MIDDLE_DOWN, [this](wxMouseEvent &) { zoom_to_selection(); });
}

void SoundPlot::OnStartSelection(wxMouseEvent &e)
{
	// Erase the anchor and the previous selection (if any) and start a (new) selection.
	m_sel_state = SelectionState::Started;
	auto pos = e.GetPosition();
	m_sel_start = pos.x;
	update_selection(std::make_pair(-1.0, -1.0));
	update_anchor(std::make_pair(-1, -1.0));
	e.Skip();
}

void SoundPlot::OnEndSelection(wxMouseEvent &e)
{
	m_sel_state = SelectionState::Inactive;
	auto pos = e.GetPosition();
	if (pos.x == m_sel_start)
	{
		// Set an anchor if the mouse button was released in the same location that it was clicked.
		if (m_track_mouse) {
			update_anchor(std::make_pair(pos.x, XPosToTime(pos.x)));
		}
		update_selection(PixelSelection{-1, -1});
		Refresh();
	}
	m_sel_start = -1;
	e.Skip();
}

void SoundPlot::OnMotion(wxMouseEvent &e)
{
	if (m_sel_state == SelectionState::Started) {
		m_sel_state = SelectionState::Active;
	}

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
	m_sel_state = SelectionState::Inactive;
	m_sel_start = -1;
	update_cursor(-1);
	e.Skip();
}

PixelSelection SoundPlot::GetSelection() const
{
	return m_sel;
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
	    auto t1 = ClipTime(XPosToTime(m_sel.first));
	    auto t2 = ClipTime(XPosToTime(m_sel.second));
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
    if (m_window.second < m_sound->duration())
    {
        auto delta = std::max<double>(GetWindowDuration() / 10, 0.001);
        auto t1 = ClipTime(m_window.first + delta);
        auto t2 = ClipTime(m_window.second + delta);
        SetTimeWindow(TimeSpan{t1, t2});
    }
}

void SoundPlot::MoveBackward()
{
    // Slide by 10%
    if (m_window.first >= 0)
    {
        auto delta = std::max<double>(GetWindowDuration() / 10, 0.001);
        auto t1 = ClipTime(m_window.first - delta);
        auto t2 = ClipTime(m_window.second - delta);
        SetTimeWindow(TimeSpan{t1, t2});
    }
}

void SoundPlot::EnableMouseTracking(bool value)
{
	m_track_mouse = value;

	if (!value)
	{
		m_anchor = {-1, -1.0};
		Refresh(); // erase anchor if there's one.
	}
}

void SoundPlot::OnMouseWheel(wxMouseEvent &e)
{
	auto steps = e.GetWheelRotation() / e.GetWheelDelta();
	auto count = std::abs(steps);

	for (int i = 1; i <= count; i++)
	{
		if (steps > 0) {
			update_window(ComputeZoomIn());
		}
		else {
			update_window(ComputeZoomOut());
		}
	}
}

TimeSpan SoundPlot::ComputeZoomIn() const
{
	// Zoom in by 25% on each side
	auto zoom = GetWindowDuration() / 4;
	auto t1 = ClipTime(m_window.first + zoom);
	auto t2 = ClipTime(m_window.second - zoom);

	return TimeSpan{t1, t2};
}

TimeSpan SoundPlot::ComputeZoomOut() const
{
	// Zoom out by 50%
	auto zoom = GetWindowDuration() / 2;
	auto t1 = ClipTime(m_window.first - zoom);
	auto t2 = ClipTime(m_window.second + zoom);

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


	path.MoveToPoint(m_sel.first, 0.0);
	path.AddLineToPoint(m_sel.second, 0.0);
	path.AddLineToPoint(m_sel.second, height);
	path.AddLineToPoint(m_sel.first, height);
	path.AddLineToPoint(m_sel.first, 0.0);
	wxBrush brush;
	brush.SetColour(PLOT_SEL_COLOUR);
	gc->SetBrush(brush);
	gc->FillPath(path);
}

void SoundPlot::DrawTimeAnchor(wxPaintDC &dc)
{
	if (!HasTimeAnchor()) {
		return;
	}
	dc.SetPen(wxPen(ANCHOR_COLOUR));
	dc.DrawLine(m_anchor.first, 0, m_anchor.first, GetHeight());
}

void SoundPlot::DrawCursor(wxPaintDC &dc)
{
	if (m_track_mouse && HasCursor() && m_sel_state != SelectionState::Active)
	{
		dc.SetPen(wxPen(CURSOR_COLOUR));
		dc.DrawLine(m_cursor_pos, 0, m_cursor_pos, GetHeight());

		if (m_is_top)
		{
			auto time = wxString::Format("%.4f", XPosToTime(m_cursor_pos));
			auto col = dc.GetTextForeground();
			dc.SetTextForeground(CURSOR_COLOUR);
			dc.DrawText(time, m_cursor_pos + 3, 0);
			dc.SetTextForeground(col);
		}
	}
}

void SoundPlot::SetCursorPosition(double pos)
{
	m_cursor_pos = pos;
	Refresh();
}

void SoundPlot::SetAnchor(TimeAnchor anchor)
{
	m_anchor = anchor;
	Refresh();
}

void SoundPlot::MakeTop(bool value)
{
	m_is_top = value;
}

bool SoundPlot::IsTop() const
{
	return m_is_top;
}

void SoundPlot::SetTimeWindow(TimeSpan win)
{
	if (HasSelection())
	{
		auto t1 = XPosToTime(m_sel.first);
		auto t2 = XPosToTime(m_sel.second);
		t1 = (std::max)(win.first, t1);
		t2 = (std::min)(win.second, t2);
		TimeAlignedWindow::SetTimeWindow(win);
		m_sel = { TimeToXPos(t1), TimeToXPos(t2) };
		Refresh();
	}
	else
	{
		TimeAlignedWindow::SetTimeWindow(win);
	}
	Refresh();
}

} // namespace phonometrica
