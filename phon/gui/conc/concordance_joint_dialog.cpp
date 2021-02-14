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
 * Created: 14/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/button.h>
#include <wx/stattext.h>
#include <phon/gui/sizer.hpp>
#include <phon/gui/conc/concordance_joint_dialog.hpp>
#include <phon/application/project.hpp>

namespace phonometrica {

ConcordanceJointDialog::ConcordanceJointDialog(wxWindow *parent, const wxString &title) :
	wxDialog(parent, wxID_ANY, title)
{
	auto sizer = new VBoxSizer;
	sizer->Add(new wxStaticText(this, wxID_ANY, _("Label for new concordance:")), 0, wxLEFT|wxTOP, 10);
	m_text = new wxTextCtrl(this, wxID_ANY);
	sizer->Add(m_text, 0, wxEXPAND|wxALL, 10);
	sizer->Add(new wxStaticText(this, wxID_ANY, _("Other concordance:")),  0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 10);
	wxArrayString choices;
	m_items = Project::get()->concordances();

	for (auto &conc : m_items) {
		choices.Add(conc->label());
	}
	m_choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
	sizer->Add(m_choice,  0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 10);
	sizer->AddStretchSpacer();

	auto btn_sizer = new HBoxSizer;
	btn_sizer->AddStretchSpacer();
	btn_sizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")));
	btn_sizer->AddSpacer(5);
	btn_sizer->Add(new wxButton(this, wxID_OK, _("OK")));
	sizer->Add(btn_sizer, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 10);
	SetSizer(sizer);
	m_text->SetFocus();
}

wxString ConcordanceJointDialog::GetLabel() const
{
	return m_text->GetValue();
}

AutoConcordance ConcordanceJointDialog::GetConcordance() const
{
	return m_items[m_choice->GetSelection() + 1];
}
} // namespace phonometrica
