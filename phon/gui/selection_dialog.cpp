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
 * Created: 21/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/stattext.h>
#include <phon/gui/sizer.hpp>
#include <phon/gui/selection_dialog.hpp>
#include <phon/gui/dialog_buttons.hpp>
#include <phon/string.hpp>

namespace phonometrica {

SelectionDialog::SelectionDialog(wxWindow *parent) :
	wxDialog(parent, wxID_ANY, _("Set selection window..."), wxDefaultPosition, wxSize(300, 220), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	auto sizer = new VBoxSizer;
	sizer->Add(new wxStaticText(this, wxID_ANY, _("From (seconds):")), 0, wxEXPAND|wxALL, 10);
	from_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	sizer->Add(from_ctrl, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);
	sizer->Add(new wxStaticText(this, wxID_ANY, _("To (seconds):")), 0, wxEXPAND|wxALL, 10);
	to_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	sizer->Add(to_ctrl, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);
	sizer->AddStretchSpacer();
	sizer->Add(new DialogButtons(this), 0, wxEXPAND | wxALL, 10);
	SetSizer(sizer);

	from_ctrl->Bind(wxEVT_TEXT_ENTER, &SelectionDialog::OnEnter, this);
	to_ctrl->Bind(wxEVT_TEXT_ENTER, &SelectionDialog::OnEnter, this);
}

TimeWindow SelectionDialog::GetSelection() const
{
	auto t1 = ParseNumber(from_ctrl);
	auto t2 = ParseNumber(to_ctrl);

	return TimeWindow{t1, t2};
}

double SelectionDialog::ParseNumber(wxTextCtrl *ctrl) const
{
	String text = ctrl->GetValue();
	bool ok;
	auto value = text.to_float(&ok);

	if (!ok) {
		throw error("Could not convert string \"%\" to number");
	}

	return value;
}

void SelectionDialog::OnEnter(wxCommandEvent &)
{
	EndModal(wxID_OK);
}

} // namespace phonometrica
