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
	TextQueryEditor(parent, make_handle<Query>(nullptr, String()))
{

}

TextQueryEditor::TextQueryEditor(wxWindow *parent, Handle<Query> q) :
	QueryEditor(parent, _("Find in annotations...")), query(std::move(q))
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
	con->search_ctrl->Bind(wxEVT_TEXT,  &TextQueryEditor::OnEnableSaving, this);
	con->search_ctrl->Bind(wxEVT_SEARCH, &TextQueryEditor::OnSearch, this);
	con->layer_ctrl->Bind(wxEVT_TEXT, &TextQueryEditor::OnEnableSaving, this);
	con->relation_selector->Bind(wxEVT_CHOICE, &TextQueryEditor::OnEnableSaving, this);
	con->case_checkbox->Bind(wxEVT_CHECKBOX, &TextQueryEditor::OnEnableSaving, this);
	con->operator_selector->Bind(wxEVT_CHOICE, &TextQueryEditor::OnEnableSaving, this);

    add_constraint_btn = new wxButton(constraint_box, wxID_ANY, wxEmptyString);
	remove_constraint_btn = new wxButton(constraint_box, wxID_ANY, wxEmptyString);
	add_constraint_btn->SetBitmap(wxBITMAP_PNG_FROM_DATA(plus));
	remove_constraint_btn->SetBitmap(wxBITMAP_PNG_FROM_DATA(minus));
	remove_constraint_btn->Enable(false);
#if __WXMAC__
	wxSize btn_size(30, -1);
#else
	wxSize btn_size(40, -1);
#endif
	add_constraint_btn->SetMaxClientSize(btn_size);
	remove_constraint_btn->SetMaxClientSize(btn_size);
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
	context_spinctrl = new wxSpinCtrl(context_box, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS); //, 1, 100, 40);
	context_spinctrl->SetToolTip(_("Number of characters in the left and right contexts"));
	context_spinctrl->SetRange(1, 1000);
	context_spinctrl->SetValue(Settings::get_int("concordance", "context_length"));
	auto ref_label = new wxStaticText(context_box, wxID_ANY, _("Reference constraint:"));
	ref_spinctrl = new wxSpinCtrl(context_box, wxID_ANY, "1");
	ref_spinctrl->SetToolTip(_("Select the constraint for which the context should be extracted"));
	ref_spinctrl->SetRange(1, 1);

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
	ctx_sizer->Add(context_spinctrl, 0, wxALL|wxALIGN_CENTER, 5);
	ctx_sizer->AddStretchSpacer();


	auto vsizer = new wxBoxSizer(wxVERTICAL);
#ifdef __WXMSW__
	vsizer->AddSpacer(30);
#endif
	vsizer->Add(ctx_sizer, 1, wxEXPAND, 0);
	vsizer->AddSpacer(20);
	context_box->SetSizer(vsizer);

#ifdef __WXGTK__
	const int con_prop = 0;
	const int ctx_prop = 0;
#else
	const int con_prop = 2;
	const int ctx_prop = 1;
#endif

	auto sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(constraint_box, con_prop, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	sizer->Add(context_box, ctx_prop, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	panel->SetSizer(sizer);
	constraints.first()->search_ctrl->SetFocus();

	return panel;
}

Handle<Query> TextQueryEditor::GetQuery() const
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

	// Load meta-constraints.
	for (auto &mc : query->metaconstraints())
	{
		if (auto desc = dynamic_cast<DescMetaConstraint*>(mc.get()))
		{
			int op = static_cast<int>(desc->op);
			desc_op_choice->SetSelection(op);
			desc_ctrl->ChangeValue(desc->value);
		}
		else if (auto cons = dynamic_cast<TextMetaConstraint*>(mc.get()))
		{
			for (auto box : properties)
			{
				if (box->GetCategory() == cons->category)
				{
					if (box->GetType() != typeid(String))
					{
						wxMessageBox(_("Invalid text property"), _("Invalid property"), wxICON_ERROR);
					}
					else
					{
						box->CheckValues(cons->values);
						break;
					}
				}
			}
		}
		else if (auto cons = dynamic_cast<NumericMetaConstraint*>(mc.get()))
		{
			for (auto box : properties)
			{
				if (box->GetCategory() == cons->category)
				{
					if (box->GetType() != typeid(double))
					{
						wxMessageBox(_("Invalid numeric property"), _("Invalid property"), wxICON_ERROR);
					}
					else
					{
						int sel = (cons->op == NumericMetaConstraint::Operator::None) ? -1 : static_cast<int>(cons->op);
						box->SetNumericValue(sel, cons->value);
					}
				}
			}
		}
		else if (auto cons = dynamic_cast<BooleanMetaConstraint*>(mc.get()))
		{
			for (auto box : properties)
			{
				if (box->GetCategory() == cons->category)
				{
					if (box->GetType() != typeid(bool))
					{
						wxMessageBox(_("Invalid Boolean property"), _("Invalid property"), wxICON_ERROR);
					}
					else
					{
						box->SetBoolean(cons->value);
					}
				}
			}
		}
		else
		{
			wxMessageBox(_("Invalid property type: this should never happen!"), _("Invalid property"), wxICON_ERROR);
		}
	}

	// Set file selection
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

	// Load constraints.
	int count = (int)query->constraint_count();
	for (int i = 1; i < count; i++)
	{
		wxCommandEvent e;
		OnAddConstraint(e);
	}
	for (int i = 1; i <= count; i++)
	{
		constraints[i]->LoadConstraint(query->get_constraint(i));
	}

	// Context
	switch (query->context())
	{
		case Query::Context::Labels:
		{
			ctx_btn2->SetValue(true);
			ref_spinctrl->SetValue(query->reference_constraint());
		} break;
		case Query::Context::KWIC:
		{
			ctx_btn3->SetValue(true);
			ref_spinctrl->SetValue(query->reference_constraint());
			context_spinctrl->SetValue(query->context_length());
		} break;
		default:
		{
			ctx_btn1->SetValue(true);
		}
	}

	EnableSaving(false);
}

void TextQueryEditor::ParseQuery()
{
	assert(query);
	if (!save_btn->IsEnabled()) {
		return; // query has not been modified, nothing to do.
	}
	query->clear();
	query->set_label(name_ctrl->GetValue(), false);

	// Meta-constraints
	if (!desc_ctrl->IsEmpty())
	{
		auto op = static_cast<DescMetaConstraint::Operator>(desc_op_choice->GetSelection());
		auto mc = std::make_shared<DescMetaConstraint>(op, desc_ctrl->GetValue());
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
			query->add_metaconstraint(std::make_shared<BooleanMetaConstraint>(category, value));
		}
		else if (type == typeid(double))
		{
			auto value = prop->GetNumericValue();
			auto op = static_cast<NumericMetaConstraint::Operator>(prop->GetOperator());
			query->add_metaconstraint(std::make_shared<NumericMetaConstraint>(category, op, value));
		}
		else
		{
			auto values = prop->GetTextValues();
			query->add_metaconstraint(std::make_shared<TextMetaConstraint>(category, values));
		}
	}

	// Context options
	if (ctx_btn1->GetValue())
	{
		query->set_context(Query::Context::None);
	}
	if (ctx_btn2->GetValue())
	{
		query->set_context(Query::Context::Labels);
		query->set_reference_constraint(ref_spinctrl->GetValue());
	}
	else if (ctx_btn3->GetValue())
	{
		query->set_context(Query::Context::KWIC);
		query->set_context_length(context_spinctrl->GetValue());
		// Remember last choice
		Settings::set_value("concordance", "context_length", intptr_t(context_spinctrl->GetValue()));
		query->set_reference_constraint(ref_spinctrl->GetValue());
	}

	// File selection, if any
	Array<Handle<Annotation>> annotations;
	for (unsigned int i = 0; i < file_list->GetCount(); i++)
	{
		if (file_list->IsChecked(i))
		{
			auto &path = file_list->GetToolTip(i);
			annotations.append(recast<Annotation>(Project::get()->get(path)));
		}
	}
	query->set_selection(std::move(annotations));

	// Constraints
	for (auto &ctrl : constraints)
	{
		query->add_constraint(ctrl->ParseConstraint());
	}
}

void TextQueryEditor::OnAddConstraint(wxCommandEvent &)
{
	auto con = new ConstraintCtrl(constraint_box, (int)constraints.size()+1, false);
	constraint_sizer->Insert(size_t(constraints.size()), con, 0, wxEXPAND);
	this->Layout();
	constraints.last()->EnableRelation(true);
	constraints.append(con);
	remove_constraint_btn->Enable();
	ref_spinctrl->SetRange(1, (int)constraints.size());
	con->search_ctrl->Bind(wxEVT_TEXT, &TextQueryEditor::OnEnableSaving, this);
	con->search_ctrl->Bind(wxEVT_SEARCH, &TextQueryEditor::OnSearch, this);
	con->case_checkbox->Bind(wxEVT_CHECKBOX, &TextQueryEditor::OnEnableSaving, this);
	con->operator_selector->Bind(wxEVT_CHOICE, &TextQueryEditor::OnEnableSaving, this);
	con->layer_ctrl->Bind(wxEVT_TEXT, &TextQueryEditor::OnEnableSaving, this);
	con->relation_selector->Bind(wxEVT_CHOICE, &TextQueryEditor::OnEnableSaving, this);
}

void TextQueryEditor::OnRemoveConstraint(wxCommandEvent &)
{
	auto con = constraints.take_last();
	constraint_sizer->Detach(con);
	delete con;
	this->Layout();
	constraints.last()->EnableRelation(false);
	remove_constraint_btn->Enable(constraints.size() != 1);
	ref_spinctrl->SetRange(1, (int)constraints.size());
}

void TextQueryEditor::OnSearch(wxCommandEvent &)
{
    for (auto &con : constraints)
    {
        if (con->search_ctrl->GetValue().IsEmpty())
        {
            wxMessageBox(_("Cannot run query with empty search field"), _("Invalid query"), wxICON_ERROR);
            return;
        }
    }
    EndModal(wxID_OK);
}

void TextQueryEditor::OnEnableSaving(wxCommandEvent &)
{
	EnableSaving(true);
}

} // namespace phonometrica
