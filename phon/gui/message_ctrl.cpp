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
	status_ctrl = new wxStaticText(this, wxID_ANY, wxString());
	sel_ctrl = new wxStaticText(this, wxID_ANY, wxString());
	layer_ctrl = new wxStaticText(this, wxID_ANY, wxString(), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxST_NO_AUTORESIZE);
	sizer->Add(status_ctrl, 1, wxEXPAND | wxALL, 0);
	sizer->Add(sel_ctrl, 1, wxEXPAND | wxALL, 0);
	sizer->Add(layer_ctrl, 1, wxEXPAND | wxALL, 0);
	SetSizer(sizer);
}

void MessageCtrl::SetStatus(const wxString &text)
{
	status_ctrl->SetLabel(text);
}

void MessageCtrl::ClearStatus()
{
	SetStatus(wxString());
}

void MessageCtrl::SetSelection(const wxString &text)
{
	sel_ctrl->SetLabel(text);
}

void MessageCtrl::ClearSelection()
{
	SetSelection(wxString());
}

void MessageCtrl::SetLayerInfo(const wxString &text)
{
	layer_ctrl->SetLabel(text);
}

void MessageCtrl::ClearLayerInfo()
{
	SetLayerInfo(wxString());
}
} // namespace phonometrica
