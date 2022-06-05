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
 * Created: 11/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <phon/gui/tool_bar.hpp>
#include <phon/include/icons.hpp>

namespace phonometrica {

static const int padding = 5;

ToolBar::ToolBar(wxWindow *parent) :
	wxWindow(parent, wxID_ANY), button_size(28, 28)
{
	inner_sizer = new HBoxSizer;
	auto sizer = new VBoxSizer;
	sizer->Add(inner_sizer, 1, wxEXPAND|wxALL, 10);
	SetSizer(sizer);
	dropdown_bmp = wxBITMAP_PNG_FROM_DATA(dropdown);
    transparent_img = wxBITMAP_PNG_FROM_DATA(transparent).ConvertToImage();
	Bind(wxEVT_PAINT, &ToolBar::OnPaint, this);
}

void ToolBar::AddSeparator()
{
	auto sep = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxSize(1, 28));
	sep->SetBackgroundColour(*wxLIGHT_GREY);
	inner_sizer->Add(sep, 0, wxLEFT|wxALIGN_CENTER, padding);
}

void ToolBar::AddStretchableSpace()
{
	inner_sizer->AddStretchSpacer();
}

wxButton *ToolBar::AddButton(const wxBitmap &bitmap, const wxString &tooltip, int id)
{
	auto btn = new wxButton(this, id, "", wxDefaultPosition, button_size, wxBORDER_NONE|wxBU_NOTEXT);
    btn->SetBitmap(MakeImage(bitmap));
    btn->SetToolTip(tooltip);
	inner_sizer->Add(btn, 0, wxLEFT, padding);

#ifndef __WXGTK__
	btn->Bind(wxEVT_ENTER_WINDOW, [btn,this](wxMouseEvent &) { btn->SetBackgroundColour(GetHoverColour()); btn->Refresh(); });
	btn->Bind(wxEVT_LEAVE_WINDOW, [btn,this](wxMouseEvent &) { btn->SetBackgroundColour(GetBackgroundColour()); btn->Refresh(); });
#endif

	return btn;
}

ToggleButton *ToolBar::AddToggleButton(const wxBitmap &bitmap, const wxString &tooltip, int id)
{
	auto btn = new ToggleButton(this, id, MakeImage(bitmap), wxDefaultPosition, button_size);
	btn->SetToolTip(tooltip);
	inner_sizer->Add(btn, 0, wxLEFT, padding);

#ifndef __WXGTK__
	btn->Bind(wxEVT_ENTER_WINDOW, [btn,this](wxMouseEvent &) { btn->SetBackgroundColour(GetHoverColour()); btn->Refresh(); });
	btn->Bind(wxEVT_LEAVE_WINDOW, [btn](wxMouseEvent &) { btn->RestoreBackgroundColour(); btn->Refresh(); });
#endif

	return btn;
}

wxButton *ToolBar::AddHelpButton()
{
#ifdef __WXMAC__
	auto btn = new wxButton(this, wxID_HELP);
	inner_sizer->Add(btn, 0, wxLEFT, padding);
	btn->SetToolTip(_("Help"));

	return btn;
#else
	return AddButton(wxBITMAP_PNG_FROM_DATA(question), _("Help"));
#endif
}

void ToolBar::ShowMenu(wxButton *button, wxMenu *menu)
{
	auto pos = ClientToScreen(button->GetPosition());
	pos.y +=  button->GetSize().GetHeight();
	this->PopupMenu(menu, ScreenToClient(pos));
}

wxButton *ToolBar::AddMenuButton(const wxBitmap &bitmap, const wxString &tooltip, int id)
{
	wxMemoryDC dc;
	auto bmp = bitmap;
	dc.SelectObject(bmp);
	dc.DrawBitmap(dropdown_bmp, 4, 4); // fit a 20x20 arrow in a 28x28 bitmap
	dc.SelectObject(wxNullBitmap);

	return AddButton(bmp, tooltip, id);
}

void ToolBar::AddSpacer(int space)
{
	inner_sizer->AddSpacer(space);
}

wxColour ToolBar::GetHoverColour() const
{
	auto col = GetBackgroundColour();
	double factor = 0.85;
	auto r = col.Red();
	auto g = col.Green();
	auto b = col.Blue();
	r = (unsigned char)(r + factor * (255 - r));
	g = (unsigned char)(g + factor * (255 - g));
	b = (unsigned char)(b + factor * (255 - b));

	return {r,g,b};
}

void ToolBar::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);
	dc.SetPen(wxPen(*wxLIGHT_GREY));
	auto size = GetSize();
	dc.DrawLine(0, 0, size.x, 0);
	dc.DrawLine(0, size.y-1, size.x, size.y-1);
	dc.DrawLine(0, 0, 0, size.y);
	dc.DrawLine(size.x-1, 0, size.x-1, size.y);

	e.Skip();
}

wxImage ToolBar::MakeImage(const wxBitmap &bitmap) const
{
    // The behaviour of buttons varies across platforms, and we can't get a nicely centered bitmap on mac if the
    // button is larger than the bitmap, so we create our own bitmap with the original bitmap centered on a
    // transparent background
    wxImage img(button_size, false);
    img.Paste(transparent_img, 0, 0);
    img.Paste(bitmap.ConvertToImage(), 2, 2);

    return img;
}

} // namespace phonometrica
