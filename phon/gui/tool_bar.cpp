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
#include <phon/gui/sizer.hpp>
#include <phon/gui/tool_bar.hpp>
#include <phon/include/icons.hpp>

namespace phonometrica {


ToolBar::ToolBar(wxWindow *parent) :
	wxWindow(parent, wxID_ANY), button_size(30, 30)
{
	SetSizer(new HBoxSizer);
}

void ToolBar::AddSeparator()
{
	GetSizer()->AddSpacer(5);
}

void ToolBar::AddStretchableSpace()
{
	GetSizer()->AddStretchSpacer();
}

wxButton *ToolBar::AddButton(const wxBitmap &bitmap, const wxString &tooltip, int id)
{
	auto btn = new wxButton(this, id, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	btn->SetBitmap(bitmap);
	btn->SetMaxSize(button_size);
	btn->SetToolTip(tooltip);
	GetSizer()->Add(btn, 0, wxLEFT, 10);

	return btn;
}

wxToggleButton *ToolBar::AddToggleButton(const wxBitmap &bitmap, const wxString &tooltip, int id)
{
	auto btn = new wxToggleButton(this, id, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	btn->SetBitmap(bitmap);
	btn->SetMaxSize(button_size);
	btn->SetToolTip(tooltip);
	GetSizer()->Add(btn, 0, wxLEFT, 10);

	return btn;
}

wxButton *ToolBar::AddHelpButton()
{
	return AddButton(wxBITMAP_PNG_FROM_DATA(question), _("Help"));
}
} // namespace phonometrica
