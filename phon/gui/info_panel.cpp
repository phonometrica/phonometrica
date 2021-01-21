/***********************************************************************************************************************
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
 * Created: 13/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/info_panel.hpp>

namespace phonometrica {

InfoPanel::InfoPanel(Runtime &rt, wxWindow *parent) :
	wxPanel(parent), runtime(rt)
{
	SetupUi();
}

void InfoPanel::SetupUi()
{
	auto header = new wxStaticText(this, wxID_ANY, _("Information"), wxDefaultPosition, wxDefaultSize);
	header->SetForegroundColour(wxColor(75, 75, 75));
	auto sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(header, 1, wxEXPAND|wxTOP|wxLEFT, 7);
	sizer->AddStretchSpacer();
	auto filler = new wxStaticText(this, wxID_ANY, _("No metadata to display"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
	sizer->Add(filler, 1, wxEXPAND, 0);
	sizer->AddStretchSpacer(2);
	SetSizer(sizer);
	auto font = header->GetFont();
	font.MakeBold();
	header->SetFont(font);
	filler->SetFont(font);
	filler->SetForegroundColour(wxColor(175, 175, 175));
}
} // namespace phonometrica
