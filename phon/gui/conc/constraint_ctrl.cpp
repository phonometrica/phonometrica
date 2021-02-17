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
#include <wx/stattext.h>
#include <phon/gui/conc/constraint_ctrl.hpp>

namespace phonometrica {

ConstraintCtrl::ConstraintCtrl(wxWindow *parent, int index, bool enable_relation) :
	wxPanel(parent, wxID_ANY)
{
#ifdef __WXGTK__
	wxSize size(-1, 30); // ensure all the controls have the same height
	wxSize layer_size(150, size.GetHeight());
#else
	auto size = wxDefaultSize;
	auto layer_size = size;
#endif
	layer_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, layer_size);
	layer_ctrl->SetToolTip(_("Leave this field empty to search anywhere, type in the index of a specific layer, or use a regular expression "
						  "to match a layer's name against"));
	SetLayerDescriptiveText(false);
	search_ctrl = new wxSearchCtrl(this, wxID_ANY, wxEmptyString);
	search_ctrl->ShowCancelButton(true);
	search_ctrl->SetDescriptiveText(_("Search target"));

	wxArrayString search_operators;
	search_operators.Add("equals");
	search_operators.Add("contains");
	search_operators.Add("matches");
	operator_selector = new wxChoice(this, wxID_ANY, wxDefaultPosition, size, search_operators);
	operator_selector->SetSelection(1);
	operator_selector->SetToolTip(_("Search an exact string (equals), a substring (contains) or a regular expression (matches)"));

	case_checkbox = new wxCheckBox(this, wxID_ANY, _("case sensitive"));

	wxArrayString relation_operators;
	relation_operators.Add("dominates");
	relation_operators.Add("strictly dominates");
	relation_operators.Add("is aligned with");
	relation_operators.Add("is left-aligned with");
	relation_operators.Add("is right-aligned with");
	relation_operators.Add("precedes");
	relation_operators.Add("follows");
	relation_selector = new wxChoice(this, wxID_ANY, wxDefaultPosition, size, relation_operators);
	relation_selector->Enable(enable_relation);
	if (enable_relation) {
		relation_selector->SetSelection(0);
	}
	relation_selector->SetToolTip(_("Relation to the following constraint (if any)"));

	auto sizer = new wxBoxSizer(wxHORIZONTAL);

	auto label = wxString::Format("%d", index);
	auto num_text = new wxStaticText (this, wxID_ANY, label, wxDefaultPosition, wxSize(25, -1));
	auto font = num_text->GetFont();
	font.MakeBold();
	num_text->SetFont(font);
	sizer->Add(num_text, 0, wxLEFT|wxTOP|wxBOTTOM|wxALIGN_CENTER, 10);
	auto txt = new wxStaticText(this, wxID_ANY, _("Layer:"));//, wxDefaultPosition, size);
	sizer->Add(txt, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER, 10);
	sizer->Add(layer_ctrl, 0, wxLEFT|wxTOP|wxBOTTOM, 10);
	sizer->Add(operator_selector, 0, wxLEFT|wxTOP|wxBOTTOM, 10);
	sizer->Add(search_ctrl, 1, wxLEFT|wxTOP|wxBOTTOM, 10);
	sizer->Add(case_checkbox, 0, wxLEFT|wxTOP|wxBOTTOM|wxALIGN_CENTER, 10);
	sizer->Add(relation_selector, 0, wxLEFT | wxTOP | wxBOTTOM | wxRIGHT, 10);
	SetSizer(sizer);

	layer_ctrl->Bind(wxEVT_SET_FOCUS, [this](wxFocusEvent &) { SetLayerDescriptiveText(true); });
	layer_ctrl->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent &) { SetLayerDescriptiveText(false); });
}

void ConstraintCtrl::EnableRelation(bool value)
{
	relation_selector->SetSelection(value ? 0 : wxNOT_FOUND);
	relation_selector->Enable(value);
}

bool ConstraintCtrl::IsCaseSensitive() const
{
	return case_checkbox->IsChecked();
}

void ConstraintCtrl::SetLayerDescriptiveText(bool focus)
{
	if (focus)
	{
		if (layer_ctrl->GetValue() == _("Index or pattern"))
		{
			layer_ctrl->SetValue(wxString());
		}
		layer_ctrl->SetForegroundColour(search_ctrl->GetForegroundColour());
	}
	else if (layer_ctrl->IsEmpty())
	{
		layer_ctrl->SetValue(_("Index or pattern"));
		layer_ctrl->SetForegroundColour(wxColor(175,175,175));
	}
}

Constraint ConstraintCtrl::ParseConstraint() const
{
	Constraint constraint;
	constraint.case_sensitive = IsCaseSensitive();
	constraint.op = GetSearchOperator();
	int sel = relation_selector->GetSelection();
	constraint.relation = (sel == wxNOT_FOUND) ? Constraint::Relation::None : static_cast<Constraint::Relation>(sel);
	constraint.target = search_ctrl->GetValue();
	String layer = layer_ctrl->GetValue();

	if (layer.empty() || layer == String(_("Index or pattern")))
	{
		constraint.layer_index = 0;
	}
	else
	{
		bool ok;
		intptr_t index = layer.to_int(&ok);
		if (ok) {
			constraint.layer_index = index;
		}
		else {
			constraint.layer_pattern = layer;
		}
	}

	return constraint;
}

void ConstraintCtrl::LoadConstraint(const Constraint &constraint)
{
	case_checkbox->SetValue(constraint.case_sensitive);
	operator_selector->SetSelection(static_cast<int>(constraint.op));
	int sel = (constraint.relation == Constraint::Relation::None) ? wxNOT_FOUND : static_cast<int>(constraint.relation);
	relation_selector->SetSelection(sel);
	if (!constraint.target.empty()) {
		search_ctrl->ChangeValue(constraint.target);
	}
	if (constraint.use_index())
	{
		if (constraint.layer_index > 0) {
			SetLayerText(wxString::Format("%d", constraint.layer_index));
		}
	}
	else
	{
		SetLayerText(constraint.layer_pattern);
	}
}

void ConstraintCtrl::SetLayerText(const wxString &text)
{
	layer_ctrl->SetForegroundColour(search_ctrl->GetForegroundColour());
	layer_ctrl->SetValue(text);
}

ConstraintCtrl::Operator ConstraintCtrl::GetSearchOperator() const
{
	return static_cast<Operator>(operator_selector->GetSelection());
}
} // namespace phonometrica
