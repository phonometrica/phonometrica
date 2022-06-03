/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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

SoundPlot::SoundPlot(wxWindow *parent, const Handle <Sound> &snd, int channel) :
		SpeechWidget(parent), m_sound(snd), m_channel(channel)
{
	Bind(wxEVT_RIGHT_DOWN, &SoundPlot::OnContextMenu, this);
	Bind(wxEVT_LEFT_DOWN, &SoundPlot::OnStartSelection, this);
	Bind(wxEVT_LEFT_UP, &SoundPlot::OnEndSelection, this);
	Bind(wxEVT_MOTION, &SoundPlot::OnMotion, this);
	Bind(wxEVT_LEAVE_WINDOW, &SoundPlot::OnLeaveWindow, this);
	Bind(wxEVT_MOUSEWHEEL, &SoundPlot::OnMouseWheel, this);
	Bind(wxEVT_MIDDLE_DOWN, [this](wxMouseEvent &) { zoom_to_selection(); });
	Bind(wxEVT_ERASE_BACKGROUND, &SoundPlot::OnEraseBackground, this);
	Bind(wxEVT_PAINT, &SoundPlot::OnPaint, this);
}

void SoundPlot::OnEraseBackground(wxEraseEvent &)
{

}

void SoundPlot::OnPaint(wxPaintEvent &)
{
	if (!HasValidCache()) {
		UpdateCache();
	}

	wxBufferedPaintDC dc(this);
	Render(dc);
}

void SoundPlot::Render(wxBufferedPaintDC &dc)
{
	assert(m_cached_bmp.IsOk());
	dc.DrawBitmap(m_cached_bmp, 0.0, 0.0, true);
	auto gc = std::unique_ptr<wxGraphicsContext>(wxGraphicsContext::Create(dc));
	if (!gc) return;

	DrawSelection(*gc);
	DrawCursor(dc, *gc);
	DrawTimeTick(*gc);
	// The Y axis is repainted before the plots, so we need to explicitly update it on repaint events,
	// otherwise the magnitude values might be incorrect.
	y_axis_modified();
}

void SoundPlot::OnStartSelection(wxMouseEvent &e)
{
	// Erase the anchor and the previous selection (if any) and start a (new) selection.
	change_selection_state(SelectionState::Started);
	auto pos = e.GetPosition();
	change_selection_start(pos.x);
	// Ensure that the cursor is shown when the user clicks on a plot.
	auto t = XPosToTime(pos.x);
	update_selection(TimeSelection{t, t});

	e.Skip();
}

void SoundPlot::OnEndSelection(wxMouseEvent &e)
{
	change_selection_state(SelectionState::Inactive);

	auto pos = e.GetPosition();
	if (pos.x == m_sel_start)
	{
		auto t = XPosToTime(m_sel_start);
		update_selection(TimeSelection{t, t});
		Refresh();
	}
	change_selection_start(-1);
	e.Skip();
}

void SoundPlot::OnMotion(wxMouseEvent &e)
{
	if (m_sel_state == SelectionState::Started) {
		change_selection_state(SelectionState::Active);
	}

	if (m_sel_start >= 0)
	{
		auto pos = e.GetPosition();
		double x1 = pos.x;
		double x2 = m_sel_start;
		if (x1 > x2) {
			std::swap(x1, x2);
		}
		double t1 = XPosToTime(x1);
		double t2 = XPosToTime(x2);

		// We don't need to refresh the plot because the signal will be sent back to us by the view
		// to which this plot is connected.
		update_selection(TimeSelection{t1, t2});
	}
	else if (m_track_mouse)
	{
		auto pos = e.GetPosition();
		update_cursor(pos.x);
	}
}

const TimeSelection & SoundPlot::GetSelection() const
{
	return m_sel;
}

void SoundPlot::SetSelection(const TimeSelection &sel)
{
	m_sel = sel;
	Refresh();
}

void SoundPlot::ZoomToSelection()
{
	if (HasSelection())
	{
		auto t1 = ClipTime(m_sel.t1);
		auto t2 = ClipTime(m_sel.t2);
		SetTimeWindow(TimeWindow{t1, t2});
	}
}

void SoundPlot::OnMouseWheel(wxMouseEvent &e)
{
	auto steps = e.GetWheelRotation() / e.GetWheelDelta();
	auto count = std::abs(steps);

	for (int i = 1; i <= count; i++)
	{
		if (steps < 0) {
			update_window(ComputeZoomIn());
		}
		else {
			update_window(ComputeZoomOut());
		}
	}
}

void SoundPlot::DrawSelection(wxGraphicsContext &gc)
{
	if (HasVisibleSelection())
	{
		if (HasPointSelection()) {
			DrawPointSelection(gc);
		}
		else {
			DrawSpanSelection(gc);
		}
	}
}

void SoundPlot::DrawSpanSelection(wxGraphicsContext &gc)
{
	auto height = GetHeight();
	auto path = gc.CreatePath();

	auto x1 = (std::max)(TimeToXPos(m_sel.t1), TimeToXPos(m_window.first));
	auto x2 = (std::min)(TimeToXPos(m_sel.t2), TimeToXPos(m_window.second));

	path.MoveToPoint(x1, 0.0);
	path.AddLineToPoint(x2, 0.0);
	path.AddLineToPoint(x2, height);
	path.AddLineToPoint(x1, height);
	path.AddLineToPoint(x1, 0.0);
	wxBrush brush;
	brush.SetColour(PLOT_SEL_COLOUR);
	gc.SetBrush(brush);
	gc.FillPath(path);
	// Draw lines
	gc.SetPen(wxPen(ANCHOR_COLOUR, 1, wxPENSTYLE_DOT));
	gc.StrokeLine(x1, 0, x1, height);
	gc.StrokeLine(x2, 0, x2, height);
}

void SoundPlot::DrawPointSelection(wxGraphicsContext &gc)
{
	if (!HasPointSelection()) {
		return;
	}
	gc.SetPen(wxPen(ANCHOR_COLOUR, 1, wxPENSTYLE_DOT));
	auto x = TimeToXPos(m_sel.t1);
	gc.StrokeLine(x, 0, x, GetHeight());
}

void SoundPlot::DrawCursor(wxBufferedPaintDC &dc, wxGraphicsContext &gc)
{
	if (m_track_mouse && HasCursor() && m_sel_state != SelectionState::Active)
	{
		gc.SetPen(wxPen(CURSOR_COLOUR));
		gc.StrokeLine(m_cursor_pos, 0, m_cursor_pos, GetHeight());

		if (m_is_top)
		{
			auto time = wxString::Format("%.4f", XPosToTime(m_cursor_pos));
			auto col = dc.GetTextForeground();
			auto sz = dc.GetTextExtent(time);
			dc.SetTextForeground(CURSOR_COLOUR);
			auto x = int(round(m_cursor_pos + 3));
			if (x + sz.x > GetWidth()) {
				x = m_cursor_pos - 3 - sz.x;
			}
			dc.DrawText(time, x, 0);
			dc.SetTextForeground(col);
		}
	}
}

void SoundPlot::SetCursorPosition(double pos)
{
	m_cursor_pos = pos;
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

void SoundPlot::InvalidateSelection()
{
	m_sel.invalidate();
	Refresh();
}

bool SoundPlot::HasVisibleSelection() const
{
	return (m_sel.t1 >= m_window.first && m_sel.t1 <= m_window.second) || (m_sel.t2 >= m_window.first && m_sel.t2 <= m_window.second);
}

void SoundPlot::SetSelectionState(SoundPlot::SelectionState value)
{
	m_sel_state = value;
}

void SoundPlot::SetSelectionStart(double value)
{
	m_sel_start = value;
}

TimeWindow SoundPlot::GetPlayWindow() const
{
	if (HasSelection() && !HasPointSelection()) {
		return {m_sel.t1, m_sel.t2};
	}

	return m_window;
}

void SoundPlot::SetTick(double time)
{
	m_tick_time = time;
	Refresh();
}

void SoundPlot::DrawTimeTick(wxGraphicsContext &gc)
{
	if (m_tick_time >= 0)
	{
		gc.SetPen(wxPen(wxColor(65, 65, 65)));
		auto x = TimeToXPos(m_tick_time);
		gc.StrokeLine(x, 0, x, GetHeight());
	}
}

void SoundPlot::UpdateSettings()
{
	ReadSettings();
	InvalidateCache();
	Refresh();
	y_axis_modified();
}

bool SoundPlot::IsChannelVisible() const
{
	return m_channel_visible;
}

void SoundPlot::SetChannelVisible(bool value)
{
	m_channel_visible = value;
}

bool SoundPlot::IsPlotVisible() const
{
	return m_plot_visible;
}

void SoundPlot::SetPlotVisible(bool value)
{
	m_plot_visible = value;
}

bool SoundPlot::IsVisible() const
{
	return IsPlotVisible() && IsChannelVisible();
}

void SoundPlot::OnContextMenu(wxMouseEvent &e)
{
	auto pos = ClientToScreen(e.GetPosition());
	request_context_menu(pos);
}

double SoundPlot::GetSelectionDuration() const
{
	return m_sel.t2 - m_sel.t1;
}

} // namespace phonometrica
