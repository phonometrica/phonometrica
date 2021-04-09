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
 * Created: 07/04/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/message_ctrl.hpp>
#include <phon/gui/sizer.hpp>

namespace phonometrica {

MessageCtrl::MessageCtrl(wxWindow *parent) : wxWindow(parent, wxID_ANY)
{
	auto sizer = new HBoxSizer;
	text_ctrl = new wxStaticText(this, wxID_ANY, wxString());
	sizer->Add(text_ctrl, 0, wxEXPAND|wxALL, 0);
	SetSizer(sizer);
}

void MessageCtrl::Print(const wxString &text)
{
	text_ctrl->SetLabel(text);
}

void MessageCtrl::Clear()
{
	Print(wxString());
}
} // namespace phonometrica
