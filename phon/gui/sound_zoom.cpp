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

#include <memory>
#include <wx/graphics.h>
#include <phon/gui/sound_zoom.hpp>

namespace phonometrica {

SoundZoom::SoundZoom(wxWindow *parent) : wxWindow(parent, wxID_ANY), m_sel({-1, -1})
{
	SetMinSize(wxSize(-1, 40));
	SetMaxSize(wxSize(-1, 40));
	// FIXME: on Windows, we need to explicitly connect EVT_ERASE_BACKGROUND to an empty slot and
	//   clear the background manually in the paint event, otherwise the zoom won't be shown unless
	//   a repaint event is triggered externally (e.g. by resizing the viewer or changing the view).
#ifdef __WXMSW__
    Bind(wxEVT_ERASE_BACKGROUND, &SoundZoom::OnEraseBackground, this);
#endif
	Bind(wxEVT_PAINT, &SoundZoom::OnPaint, this);
}

void SoundZoom::OnEraseBackground(wxEraseEvent &e)
{

}

void SoundZoom::OnPaint(wxPaintEvent &)
{
	if (!HasSelection()) {
		return;
	}
	wxClientDC dc(this);
	// See comment in the constructor.
#ifdef __WXMSW__
    dc.SetBackground(GetBackgroundColour());
    dc.Clear();
#endif
	auto gc = std::unique_ptr<wxGraphicsContext>(wxGraphicsContext::Create(dc));
	if (!gc) return;
	auto height = GetSize().GetHeight();
	auto width = GetSize().GetWidth();

	wxGraphicsPath path = gc->CreatePath();
	path.MoveToPoint(0.0, 0.0);
	path.AddLineToPoint(m_sel.first, height);
	path.AddLineToPoint(m_sel.second, height);
	path.AddLineToPoint(width, 0.0);
	path.AddLineToPoint(0.0, 0.0);
	wxBrush brush;
	brush.SetColour(WAVEBAR_SEL_COLOUR);
	gc->SetBrush(brush);
	gc->FillPath(path);
}

void SoundZoom::OnSetSelection(PixelSelection sel)
{
	m_sel = sel;
	Refresh();
    Update();
}

} // namespace phonometrica
