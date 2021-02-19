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
 * Created: 11/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <phon/gui/sizer.hpp>
#include <phon/gui/tool_bar.hpp>
#include <phon/include/icons.hpp>

namespace phonometrica {

static const int padding = 5;

ToolBar::ToolBar(wxWindow *parent) :
	wxWindow(parent, wxID_ANY), button_size(28, 28)
{
	SetSizer(new HBoxSizer);
}

void ToolBar::AddSeparator()
{
	auto sep = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxSize(1, 28));
	sep->SetBackgroundColour(*wxLIGHT_GREY);
	GetSizer()->Add(sep, 0, wxLEFT|wxALIGN_CENTER, padding);
}

void ToolBar::AddStretchableSpace()
{
	GetSizer()->AddStretchSpacer();
}

wxButton *ToolBar::AddButton(const wxBitmap &bitmap, const wxString &tooltip, int id)
{
	auto btn = new wxBitmapButton(this, id, bitmap, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	btn->SetMaxSize(button_size);
	btn->SetToolTip(tooltip);
	GetSizer()->Add(btn, 0, wxLEFT, padding);

#ifndef __WXGTK__
	btn->Bind(wxEVT_ENTER_WINDOW, [btn,this](wxMouseEvent &) { btn->SetBackgroundColour(GetHoverColour()); btn->Refresh(); });
	btn->Bind(wxEVT_LEAVE_WINDOW, [btn,this](wxMouseEvent &) { btn->SetBackgroundColour(GetBackgroundColour()); btn->Refresh(); });
#endif

	return btn;
}

ToggleButton *ToolBar::AddToggleButton(const wxBitmap &bitmap, const wxString &tooltip, int id)
{
	auto btn = new ToggleButton(this, id, bitmap, wxDefaultPosition, wxDefaultSize);
	btn->SetMaxSize(button_size);
	btn->SetToolTip(tooltip);
	GetSizer()->Add(btn, 0, wxLEFT, padding);

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
	GetSizer()->Add(btn, 0, wxLEFT, padding);
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
	// This method doesn't do anything special, but indicates that the menu has a drop-down menu
	return AddButton(bitmap, tooltip, id);
}

void ToolBar::AddSpacer(int space)
{
	GetSizer()->AddSpacer(space);
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

	return wxColour(r,g,b);
}

} // namespace phonometrica
