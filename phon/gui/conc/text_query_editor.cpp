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
 * Created: 01/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <phon/gui/conc/text_query_editor.hpp>
#include <phon/application/project.hpp>
#include <phon/application/settings.hpp>
#include <phon/include/icons.hpp>

namespace phonometrica {

TextQueryEditor::TextQueryEditor(wxWindow *parent) :
	TextQueryEditor(parent, std::make_shared<TextQuery>(nullptr, String()))
{

}

TextQueryEditor::TextQueryEditor(wxWindow *parent, AutoTextQuery q) :
	QueryEditor(parent, _("Query editor")), query(std::move(q))
{

}

wxPanel *TextQueryEditor::MakeSearchPanel(wxWindow *parent)
{
	auto panel = new wxPanel(parent, wxID_ANY);

	// Create the constraints
	constraint_box = new wxStaticBox(panel, wxID_ANY, _("Constraints"));
	constraint_sizer = new wxBoxSizer(wxVERTICAL);
	auto con = new ConstraintCtrl(constraint_box, 1, false);
	constraints.append(con);
	constraint_sizer->Add(con, 0, wxEXPAND);
	con->search_ctrl->Bind(wxEVT_TEXT, [this](wxCommandEvent &){ EnableSaving(true); });
	con->layer_ctrl->Bind(wxEVT_TEXT, [this](wxCommandEvent &){ EnableSaving(true); });
	con->relation_selector->Bind(wxEVT_CHOICE, [this](wxCommandEvent &){ EnableSaving(true); });

#ifdef __WXMAC__
	add_constraint_btn = new wxButton(constraint_box, wxID_ANY, "+");
	remove_constraint_btn = new wxButton(constraint_box, wxID_ANY, "-");
#else
	add_constraint_btn = new wxButton(constraint_box, wxID_ANY, wxEmptyString);
	remove_constraint_btn = new wxButton(constraint_box, wxID_ANY, wxEmptyString);
	add_constraint_btn->SetBitmap(wxBITMAP_PNG_FROM_DATA(plus), wxTOP);
	remove_constraint_btn->SetBitmap(wxBITMAP_PNG_FROM_DATA(minus));
#endif
	remove_constraint_btn->Enable(false);
	add_constraint_btn->SetMaxClientSize(wxSize(40, -1));
	remove_constraint_btn->SetMaxClientSize(wxSize(40, -1));
	add_constraint_btn->SetToolTip(_("Add constraint"));
	remove_constraint_btn->SetToolTip(_("Remove constraint"));
	auto btn_sizer = new wxBoxSizer(wxHORIZONTAL);
	btn_sizer->AddStretchSpacer();
	btn_sizer->Add(add_constraint_btn);
	btn_sizer->AddSpacer(5);
	btn_sizer->Add(remove_constraint_btn);
	constraint_sizer->Add(btn_sizer, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 10);
	constraint_sizer->AddSpacer(20);
	constraint_box->SetSizer(constraint_sizer);

	add_constraint_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextQueryEditor::OnAddConstraint, this);
	remove_constraint_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextQueryEditor::OnRemoveConstraint, this);

	// Create the context radio buttons
	auto context_box = new wxStaticBox(panel, wxID_ANY, _("Context"));
	ctx_btn1 = new wxRadioButton(context_box, wxID_ANY, _("No context"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	ctx_btn2 = new wxRadioButton(context_box, wxID_ANY, _("Surrounding labels"));
	ctx_btn3 = new wxRadioButton(context_box, wxID_ANY, _("Number of characters"));
	ctx_btn3->SetValue(true);
	ctx_btn1->SetToolTip(_("Don't extract any context"));
	ctx_btn2->SetToolTip(_("Extract labels from surrounding events"));
	ctx_btn3->SetToolTip(_("Extract fixed-sized left and right contexts"));
	context_spinctrl = new wxSpinCtrl(context_box, wxID_ANY);
	context_spinctrl->SetToolTip(_("Number of characters in the left and right contexts"));
	context_spinctrl->SetRange(1, 1000);
	context_spinctrl->SetValue((int)Settings::get_int("match_window_length"));
	auto ref_label = new wxStaticText(context_box, wxID_ANY, _("Reference constraint:"));
	ref_spinctrl = new wxSpinCtrl(context_box, wxID_ANY, "1");
	ref_spinctrl->SetToolTip(_("Select the constraint for which the context should be extracted"));

	ctx_btn1->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent &) { context_spinctrl->Disable(); ref_spinctrl->Disable(); EnableSaving(true); });
	ctx_btn2->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent &) { context_spinctrl->Disable(); ref_spinctrl->Enable(); EnableSaving(true); });
	ctx_btn3->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent &) { context_spinctrl->Enable(); ref_spinctrl->Enable(); EnableSaving(true); });
	context_spinctrl->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent &) { EnableSaving(true); });
	ref_spinctrl->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent &) { EnableSaving(true); });

	auto ctx_sizer = new wxBoxSizer(wxHORIZONTAL);
	ctx_sizer->AddSpacer(5);
	ctx_sizer->Add(ref_label, 0, wxALL|wxALIGN_CENTER, 5);
	ctx_sizer->Add(ref_spinctrl, 0, wxALL | wxALIGN_CENTER, 5);

	ctx_sizer->Add(ctx_btn1, 0, wxALL|wxALIGN_CENTER, 5);
	ctx_sizer->Add(ctx_btn2, 0, wxALL|wxALIGN_CENTER, 5);
	ctx_sizer->Add(ctx_btn3, 0, wxALL|wxALIGN_CENTER, 5);
	ctx_sizer->Add(context_spinctrl, 0, wxALL, 5);
	ctx_sizer->AddStretchSpacer();


	auto vsizer = new wxBoxSizer(wxVERTICAL);
	vsizer->Add(ctx_sizer, 1, wxEXPAND, 0);
	vsizer->AddSpacer(20);
	context_box->SetSizer(vsizer);



	auto sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(constraint_box, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	sizer->Add(context_box, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	panel->SetSizer(sizer);

	return panel;
}

AutoQuery TextQueryEditor::GetQuery() const
{
	return query;
}

void TextQueryEditor::LoadQuery()
{
	assert(query);
	auto label = query->label();
	if (!label.empty()) {
		name_ctrl->ChangeValue(label);
	}

	for (auto &mc : query->metaconstraints())
	{
		DescMetaConstraint *desc;

		if ((desc = dynamic_cast<DescMetaConstraint*>(mc.get())))
		{
			int op = static_cast<int>(desc->op);
			desc_op_choice->SetSelection(op);
			desc_ctrl->ChangeValue(desc->value);
		}
	}

	for (auto &file : query->selection())
	{
		bool found = false;
		for (unsigned int i = 0; i < file_list->GetCount(); i++)
		{
			if (file->path() == String(file_list->GetToolTip(i)))
			{
				file_list->Check(i);
				found = true;
				break;
			}
		}

		if (!found)
		{
			auto msg = wxString::Format(_("I couldn't find the following file: %s"), file->path().data());
			wxMessageBox(msg, _("Missing file"), wxICON_WARNING);
		}
	}
}

void TextQueryEditor::ParseQuery()
{
	assert(query);
	query->clear();
	query->set_label(name_ctrl->GetValue(), false);

	// Meta-constraints
	if (!desc_ctrl->IsEmpty() || desc_op_choice->GetSelection() != wxNOT_FOUND)
	{
		auto op = static_cast<DescMetaConstraint::Operator>(desc_op_choice->GetSelection());
		auto mc = std::make_unique<DescMetaConstraint>(op, desc_ctrl->GetValue());
		query->add_metaconstraint(std::move(mc), true);
	}

	for (auto prop : properties)
	{
		if (!prop->HasSelection()) continue;
		auto &category = prop->GetCategory();
		auto &type = prop->GetType();

		if (type == typeid(bool))
		{
			bool value = prop->GetBoolean();
			query->add_metaconstraint(std::make_unique<BooleanMetaConstraint>(category, value));
		}
		else if (type == typeid(double))
		{
			auto value = prop->GetNumericValue();
			auto op = static_cast<NumericMetaConstraint::Operator>(prop->GetOperator());
			query->add_metaconstraint(std::make_unique<NumericMetaConstraint>(category, op, value));
		}
		else
		{
			auto values = prop->GetTextValues();
			query->add_metaconstraint(std::make_unique<TextMetaConstraint>(category, values));
		}
	}

	// Context options
	if (ctx_btn2->GetValue())
	{
		query->set_context(TextQuery::Context::Labels);
		query->set_reference_constraint(ref_spinctrl->GetValue());
	}
	else if (ctx_btn3->GetValue())
	{
		query->set_context(TextQuery::Context::KWIC);
		query->set_context_length(context_spinctrl->GetValue());
		query->set_reference_constraint(ref_spinctrl->GetValue());
	}

	// File selection, if any
	Array<AutoAnnotation> annotations;
	for (unsigned int i = 0; i < file_list->GetCount(); i++)
	{
		if (file_list->IsChecked(i))
		{
			auto &path = file_list->GetToolTip(i);
			annotations.append(downcast<Annotation>(Project::get()->get(path)));
		}
	}
	query->set_selection(std::move(annotations));
}

void TextQueryEditor::OnAddConstraint(wxCommandEvent &)
{
	auto con = new ConstraintCtrl(constraint_box, (int)constraints.size()+1, false);
	constraint_sizer->Insert(size_t(constraints.size()), con, 0, wxEXPAND);
	this->Layout();
	constraints.last()->EnableRelation(true);
	constraints.append(con);
	remove_constraint_btn->Enable();
}

void TextQueryEditor::OnRemoveConstraint(wxCommandEvent &)
{
	auto con = constraints.take_last();
	constraint_sizer->Detach(con);
	delete con;
	this->Layout();
	constraints.last()->EnableRelation(false);
	remove_constraint_btn->Enable(constraints.size() != 1);
}

} // namespace phonometrica
