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

#include <wx/dcclient.h>
#include <phon/gui/y_axis_info.hpp>

#ifdef __WXMSW__
#undef DrawText
#endif

namespace phonometrica {

YAxisInfo::YAxisInfo(wxWindow *parent) : wxWindow(parent, wxID_ANY)
{
	const int width = 80;
	SetMinSize(wxSize(width, -1));
	SetMaxSize(wxSize(width, -1));
	Bind(wxEVT_PAINT, &YAxisInfo::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &YAxisInfo::OnClick, this);
}

void YAxisInfo::AddWindow(TimeAlignedWindow *win)
{
	m_windows.append(win);
}

void YAxisInfo::RemoveWindow(TimeAlignedWindow *win)
{
	m_windows.remove(win);
}

void YAxisInfo::OnPaint(wxPaintEvent &)
{
	wxPaintDC dc(this);

    for (auto win : m_windows)
    {
        if (win->IsShown())
        {
            // Get bounding rectangle for the widget on the Y axis.
            auto size = win->GetSize();
            auto pos = win->GetPosition();
            auto msize = this->GetSize();
            auto mpos = this->GetPosition();
            auto x = 0;
            auto y = pos.y - mpos.y;
			auto w = msize.GetWidth();
			auto h = size.GetHeight();
			wxRect rect(x, y, w, h);
            win->DrawYAxis(dc, rect);
        }
    }
}

void YAxisInfo::OnClick(wxMouseEvent &)
{
	invalidate_selection();
}

} // namespace phonometrica
