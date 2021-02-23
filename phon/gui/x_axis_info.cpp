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

#ifdef __WXMSW__
#undef DrawText
#endif

namespace phonometrica {

XAxisInfo::XAxisInfo(wxWindow *parent) :
	wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 20))
{
	Bind(wxEVT_PAINT, &XAxisInfo::OnPaint, this);
}

void XAxisInfo::SetTimeWindow(TimeSpan win)
{
	m_win = win;
	Refresh();
}

void XAxisInfo::OnPaint(wxPaintEvent &)
{
	if (!HasTimeWindow()) {
		return;
	}
	wxPaintDC dc(this);
	wxCoord width, height, x, y;
	auto size = GetSize();


	wxRect used_area1, used_area2;

	if (HasTimeAnchor())
	{
		auto colour = dc.GetTextForeground();
		dc.SetTextForeground(ANCHOR_COLOUR);
		auto time = wxString::Format("%.4f", m_anchor.second);
		dc.GetTextExtent(time, &width, &height);
		x = m_anchor.first + 3;
		y = size.y - height - 1;
		if (x + width > size.x) {
			x = size.x - width;
		}
		dc.DrawText(time, x, y);
		used_area1 = wxRect(x, y, width, height);
		dc.SetTextForeground(colour);
	}
	else if (HasSelection())
	{
		auto colour = dc.GetTextForeground();
		dc.SetTextForeground(PLOT_SEL_TEXT_COLOUR);
		auto time = wxString::Format("%.4f", m_time_sel.first);
		dc.GetTextExtent(time, &width, &height);
		x = (std::max)(0, int(m_sel.first) - width);
		y = size.y - height - 1;
		dc.DrawText(time, x, y);
		used_area1 = wxRect(x, y, width, height);

		time = wxString::Format("%.4f", m_time_sel.second);
		dc.GetTextExtent(time, &width, &height);
		int xlimit = used_area1.x + used_area1.width + 5; // 5 pixels for spacing
		x = (std::max)(xlimit, int(m_sel.second + 3));
		y = size.y - height - 1;
		dc.DrawText(time, x, y);
		used_area2 = wxRect(x, y, width, height);

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

void XAxisInfo::SetAnchor(TimeAnchor anchor)
{
	m_anchor = anchor;
	Refresh();
}

void XAxisInfo::SetSelection(PixelSelection sel, TimeSpan tsel)
{
	m_sel = sel;
	m_time_sel = tsel;
	Refresh();
}

} // namespace phonometrica
