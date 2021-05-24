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
 * Created: 22/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/x_axis_info.hpp>
#include <phon/gui/plot/time_window.hpp> // for PaintDC

#ifdef __WXMSW__
#undef DrawText
#endif

namespace phonometrica {

XAxisInfo::XAxisInfo(wxWindow *parent) :
	wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 20))
{
    //Bind(wxEVT_ERASE_BACKGROUND, &XAxisInfo::OnEraseBackground, this);
	Bind(wxEVT_PAINT, &XAxisInfo::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &XAxisInfo::OnClick, this);
}

void XAxisInfo::SetTimeWindow(TimeWindow win)
{
	m_win = win;
	Refresh();
}
void XAxisInfo::OnEraseBackground(wxEraseEvent &)
{

}

void XAxisInfo::OnPaint(wxPaintEvent &)
{
	if (!HasTimeWindow()) {
		return;
	}
    PaintDC dc(this);
#ifdef __WXMSW__
    dc.SetBackground(GetBackgroundColour());
    dc.Clear();
#endif
	wxCoord width, height, x, y;
	auto size = GetSize();
	auto x1 = TimeToXPos(m_sel.t1);
	auto x2 = TimeToXPos(m_sel.t2);
	wxRect used_area1, used_area2;

	if (HasSelection())
	{
		auto colour = dc.GetTextForeground();
		dc.SetTextForeground(ANCHOR_COLOUR);

		if (HasPointSelection())
		{
			auto time = wxString::Format("%.4f", m_sel.t1);
			dc.GetTextExtent(time, &width, &height);
			x = x1 + 3;
			y = size.y - height - 1;
			if (x + width > size.x) {
				x = size.x - width;
			}
			dc.DrawText(time, x, y);
			used_area1 = wxRect(x, y, width, height);
		}
		else
		{
			auto time = wxString::Format("%.4f", m_sel.t1);
			dc.GetTextExtent(time, &width, &height);
			x = (std::max)(0, int(x1) - width);
			y = size.y - height - 1;
			dc.DrawText(time, x, y);
			used_area1 = wxRect(x, y, width, height);

			time = wxString::Format("%.4f", m_sel.t2);
			dc.GetTextExtent(time, &width, &height);
			int xlimit = used_area1.x + used_area1.width + 5; // 5 pixels for spacing
			x = (std::max)(xlimit, int(x2 + 3));
			y = size.y - height - 1;
			dc.DrawText(time, x, y);
			used_area2 = wxRect(x, y, width, height);
		}
		dc.SetTextForeground(colour);
	}

	auto from = wxString::Format("%.4f", m_win.first);
	dc.GetTextExtent(from, &width, &height);
	x = 0;
	y = size.y - height - 1;
	wxRect rect(x, y, width, height);

	if (!used_area1.Intersects(rect) && !used_area2.Intersects(rect)) {
		dc.DrawText(from, x, y);
	}

	auto to = wxString::Format("%.4f", m_win.second);
	dc.GetTextExtent(to, &width, &height);
	x = size.x - width;
	y = size.y - height - 1;
	rect = wxRect(x, y, width, height);

	if (!used_area1.Intersects(rect) && !used_area2.Intersects(rect)) {
		dc.DrawText(to, x, y);
	}
}

void XAxisInfo::SetSelection(const TimeSelection &sel)
{
	m_sel = sel;
	Refresh();
}

double XAxisInfo::TimeToXPos(double t) const
{
	return (t - m_win.first) * GetSize().GetWidth() / (m_win.second - m_win.first);
}

void XAxisInfo::OnClick(wxMouseEvent &)
{
	invalidate_selection();
}

} // namespace phonometrica
