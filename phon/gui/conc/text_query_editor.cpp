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
	auto ctx_btn1 = new wxRadioButton(context_box, wxID_ANY, _("No context"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	auto ctx_btn2 = new wxRadioButton(context_box, wxID_ANY, _("Surrounding labels"));
	auto ctx_btn3 = new wxRadioButton(context_box, wxID_ANY, _("Number of characters"));
	ctx_btn3->SetValue(true);
	ctx_btn1->SetToolTip(_("Don't extract any context"));
	ctx_btn2->SetToolTip(_("Extract labels from surrounding events"));
	ctx_btn3->SetToolTip(_("Extract fixed-sized left and right contexts"));
	auto context_spinctrl = new wxSpinCtrl(context_box, wxID_ANY);
	context_spinctrl->SetToolTip(_("Number of characters in the left and right contexts"));
	context_spinctrl->SetRange(1, 1000);
	context_spinctrl->SetValue((int)Settings::get_int("match_window_length"));
	auto ref_label = new wxStaticText(context_box, wxID_ANY, _("Reference constraint:"));
	auto ref_spin = new wxSpinCtrl(context_box, wxID_ANY, "1");
	ref_spin->SetToolTip(_("Select the constraint for which the context should be extracted"));

	ctx_btn1->Bind(wxEVT_RADIOBUTTON, [=](wxCommandEvent &) { context_spinctrl->Disable(); ref_spin->Disable(); });
	ctx_btn2->Bind(wxEVT_RADIOBUTTON, [=](wxCommandEvent &) { context_spinctrl->Enable(); ref_spin->Enable(); });
	ctx_btn3->Bind(wxEVT_RADIOBUTTON, [=](wxCommandEvent &) { context_spinctrl->Enable(); ref_spin->Enable(); });

	auto ctx_sizer = new wxBoxSizer(wxHORIZONTAL);
	ctx_sizer->AddSpacer(5);
	ctx_sizer->Add(ref_label, 0, wxALL|wxALIGN_CENTER, 5);
	ctx_sizer->Add(ref_spin, 0, wxALL|wxALIGN_CENTER, 5);

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

	if (!desc_ctrl->IsEmpty() || desc_op_choice->GetSelection() != wxNOT_FOUND)
	{
		auto op = static_cast<DescMetaConstraint::Operator>(desc_op_choice->GetSelection());
		auto mc = std::make_unique<DescMetaConstraint>(op, desc_ctrl->GetValue());
		query->add_metaconstraint(std::move(mc), true);
	}

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
	auto con = new ConstraintCtrl(constraint_box, constraints.size()+1, false);
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
