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
#include <wx/stattext.h>
#include <phon/gui/conc/constraint_ctrl.hpp>

namespace phonometrica {

ConstraintCtrl::ConstraintCtrl(wxWindow *parent, int index, bool enable_relation) :
	wxPanel(parent, wxID_ANY)
{
	wxSize size(-1, 30); // ensure all the controls have the same height
	layer_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(150, size.GetHeight()));
	layer_ctrl->SetValue(_("index or pattern"));
	layer_ctrl->SetForegroundColour(wxColor(150,150,150));
	layer_ctrl->SetToolTip(_("Leave this field empty to search anywhere, type in the index of a specific layer, or use a regular expression "
						  "to match a layer's name against"));
	search_ctrl = new wxSearchCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, size, wxTE_PROCESS_ENTER);
	search_ctrl->ShowCancelButton(true);
	search_ctrl->SetDescriptiveText(_("Search text or pattern"));
	search_ctrl->SetToolTip(_("Search plain text or a regular expression (click on the magnifying glass to change search options)"));
	auto menu = new wxMenu;
	auto regex_id = wxNewId();
	regex_entry = menu->AppendCheckItem(regex_id, _("Use regular expressions"));
	regex_entry->Check();
	auto case_id = wxNewId();
	case_entry = menu->AppendCheckItem(case_id, _("Case-sensitive"));
	search_ctrl->SetMenu(menu);

	wxArrayString operators;
	operators.Add("dominates");
	operators.Add("strictly dominates");
	operators.Add("is left-aligned with");
	operators.Add("is right-aligned with");
	operators.Add("precedes");
	operators.Add("follows");
	relation_selector = new wxChoice(this, wxID_ANY, wxDefaultPosition, size, operators);
	relation_selector->Enable(enable_relation);
	if (enable_relation) {
		relation_selector->SetSelection(0);
	}
	relation_selector->SetToolTip(_("Relation to the following constraint (if any)"));

	auto sizer = new wxBoxSizer(wxHORIZONTAL);

	auto label = wxString::Format("%d", index);
	auto num_text = new wxStaticText (this, wxID_ANY, label, wxDefaultPosition, wxSize(25, size.GetHeight()));
	auto font = num_text->GetFont();
	font.MakeBold();
	num_text->SetFont(font);
	sizer->Add(num_text, 0, wxLEFT|wxTOP|wxALIGN_CENTER, 10);
	auto txt = new wxStaticText(this, wxID_ANY, _("Layer:"), wxDefaultPosition, size);
	sizer->Add(txt, 0, wxTOP|wxALIGN_CENTER, 10);
	sizer->Add(layer_ctrl, 0, wxLEFT|wxTOP, 10);
	sizer->Add(search_ctrl, 1, wxLEFT|wxTOP|wxBOTTOM, 10);
	sizer->Add(relation_selector, 0, wxLEFT | wxTOP | wxRIGHT, 10);
	SetSizer(sizer);
}

void ConstraintCtrl::EnableRelation(bool value)
{
	relation_selector->SetSelection(value ? 0 : wxNOT_FOUND);
	relation_selector->Enable(value);
}

bool ConstraintCtrl::UsesRegex() const
{
	return regex_entry->IsChecked();
}

bool ConstraintCtrl::IsCaseSensitive() const
{
	return case_entry->IsChecked();
}
} // namespace phonometrica
