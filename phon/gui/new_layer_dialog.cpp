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
 * Created: 11/06/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <phon/gui/new_layer_dialog.hpp>
#include <phon/gui/dialog_buttons.hpp>

namespace phonometrica {

NewLayerDialog::NewLayerDialog(wxWindow *parent, intptr_t nlayer) :
		wxDialog(parent, wxID_ANY, _("Add layer..."), wxDefaultPosition, wxSize(300, 200), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	auto sizer = new wxBoxSizer(wxVERTICAL);
	auto grid = new wxFlexGridSizer(3, 2, 10, 10);
	grid->AddGrowableCol(1);
	grid->Add(new wxStaticText(this, wxID_ANY, _("Name:")), 0, wxALIGN_CENTER);
	m_label_ctrl = new wxTextCtrl(this, wxID_ANY);
	grid->Add(m_label_ctrl, 0, wxEXPAND);
	grid->Add(new wxStaticText(this, wxID_ANY, _("Event type:")), 0, wxALIGN_CENTER);
	wxArrayString choices;
	choices.Add(_("intervals"));
	choices.Add(_("instants"));
	m_type_choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
	m_type_choice->SetSelection(0);
	grid->Add(m_type_choice, 0, wxEXPAND);
	grid->Add(new wxStaticText(this, wxID_ANY, _("Layer index:")), 0, wxALIGN_CENTER);
	auto pos = int(nlayer+1);
	m_index_spin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, pos);
	m_index_spin->SetValue(pos);
	grid->Add(m_index_spin, 0, wxEXPAND);
	sizer->Add(grid, 1, wxEXPAND|wxLEFT|wxTOP|wxRIGHT, 10);
	sizer->Add(new DialogButtons(this), 0, wxEXPAND | wxALL, 10);
	SetSizer(sizer);
}

wxString NewLayerDialog::GetLayerLabel() const
{
	return m_label_ctrl->GetValue();
}

bool NewLayerDialog::HasInstants() const
{
	return m_type_choice->GetSelection() == 1;
}

int NewLayerDialog::GetIndex() const
{
	return m_index_spin->GetValue();
}
} // namespace phonometrica
