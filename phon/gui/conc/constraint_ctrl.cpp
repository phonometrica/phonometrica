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
 * Created: 03/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/sizer.h>
#include <wx/menu.h>
#include <phon/gui/conc/constraint_ctrl.hpp>

namespace phonometrica {

ConstraintCtrl::ConstraintCtrl(wxWindow *parent, int style) :
	wxPanel(parent, wxID_ANY)
{
	wxArrayString choices;
	wxSize size(-1, 30);
	choices.Add("Layer index");
	choices.Add("Layer name (regex)");
	location_selector = new wxChoice(this, wxID_ANY, wxDefaultPosition, size, choices);
	location_selector->SetSelection(0);
	layer_ctrl = new wxTextCtrl(this, wxID_ANY, _("any"), wxDefaultPosition, size);
//	auto height = layer_ctrl->GetSize().GetHeight();
	//location_selector->SetMaxSize(wxSize(-1, height));
	location_selector->Disable();
	layer_spin = new wxSpinButton(this, wxID_ANY, wxDefaultPosition, size, wxSP_VERTICAL);
	layer_spin->SetRange(0, 1000);
	layer_spin->SetValue(0);
	search_ctrl = new wxSearchCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, size, wxTE_PROCESS_ENTER);
	search_ctrl->ShowCancelButton(true);
	search_ctrl->SetDescriptiveText(_("Search text or pattern"));
	auto menu = new wxMenu;
	auto regex_id = wxNewId();
	auto regex_entry = menu->AppendCheckItem(regex_id, _("Use regular expressions"));
	regex_entry->Check();
	auto case_id = wxNewId();
	auto case_entry = menu->AppendCheckItem(case_id, _("Case-sensitive"));
	search_ctrl->SetMenu(menu);

	wxArrayString operators;
	operators.Add("dominates");
	operators.Add("strictly dominates");
	operators.Add("is left-aligned with");
	operators.Add("is right-aligned with");
	operators.Add("precedes");
	operators.Add("follows");
	operator_selector = new wxChoice(this, wxID_ANY, wxDefaultPosition, size, operators);
	operator_selector->Disable();

	auto sizer = new wxBoxSizer(wxHORIZONTAL);
	auto rb = new wxRadioButton(this, wxID_ANY, wxString(), wxDefaultPosition, wxDefaultSize, style);
	rb->SetValue(style != 0);
	sizer->Add(rb, 0, wxLEFT|wxALIGN_CENTER, 5);
	sizer->Add(location_selector, 0, wxLEFT|wxALIGN_CENTER, 5);
	sizer->Add(layer_ctrl, 0, wxLEFT|wxTOP|wxALIGN_CENTER, 5);
	sizer->Add(layer_spin, 0, wxALIGN_CENTER, 0);
	sizer->Add(search_ctrl, 1, wxEXPAND|wxLEFT|wxTOP|wxBOTTOM, 10);
	sizer->Add(operator_selector, 0, wxALL|wxALIGN_CENTER, 10);
	SetSizer(sizer);
}
} // namespace phonometrica
