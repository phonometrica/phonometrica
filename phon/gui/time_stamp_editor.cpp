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
 * Created: 24/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/stattext.h>
#include <wx/button.h>
#include <phon/gui/sizer.hpp>
#include <phon/gui/time_stamp_editor.hpp>

namespace phonometrica {

TimeStampEditor::TimeStampEditor(wxWindow *parent) : wxDialog(parent, wxID_ANY, "Add bookmark...")
{
	auto sizer = new VBoxSizer;

	sizer->Add(new wxStaticText(this, wxID_ANY, _("Title:")), 0, wxALL, 10);
	title_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT);
	sizer->Add(title_ctrl, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT|wxLEFT, 10);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("Notes:")), 0, wxALL, 10);
	notes_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT|wxTE_MULTILINE);
	sizer->Add(notes_ctrl, 1, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT|wxLEFT, 10);

	auto btn_sizer = new HBoxSizer;
	btn_sizer->AddStretchSpacer();
	btn_sizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")));
	btn_sizer->AddSpacer(5);
	btn_sizer->Add(new wxButton(this, wxID_OK, _("OK")));
	sizer->Add(btn_sizer, 0, wxEXPAND|wxALL, 10);
	SetSizer(sizer);
}

wxString TimeStampEditor::GetBookmarkTitle() const
{
	return title_ctrl->GetValue();
}

wxString TimeStampEditor::GetNotes() const
{
	return notes_ctrl->GetValue();
}
} // namespace phonometrica
