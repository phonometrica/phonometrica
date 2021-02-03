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
#include <phon/gui/conc/constraint_ctrl.hpp>
#include <phon/gui/conc/text_query_editor.hpp>
#include <phon/application/project.hpp>
#include <phon/application/settings.hpp>

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

	// Create the context radio buttons
	auto context_box = new wxStaticBox(panel, wxID_ANY, _("Context"));
	auto ctx_btn1 = new wxRadioButton(context_box, wxID_ANY, _("None"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	auto ctx_btn2 = new wxRadioButton(context_box, wxID_ANY, _("Surrounding labels"));
	auto ctx_btn3 = new wxRadioButton(context_box, wxID_ANY, _("Keyword in context"));
	ctx_btn1->SetToolTip(_("Don't extract any context"));
	ctx_btn2->SetToolTip(_("Extract labels from surrounding events"));
	ctx_btn3->SetToolTip(_("Extract fixed-sized left and right contexts"));
	auto context_spinctrl = new wxSpinCtrl(context_box, wxID_ANY);
	context_spinctrl->SetToolTip(_("Length of the left and right contexts"));
	context_spinctrl->SetRange(1, 1000);
	context_spinctrl->SetValue((int)Settings::get_int("match_window_length"));
	auto ctx_sizer = new wxBoxSizer(wxHORIZONTAL);
	ctx_sizer->Add(ctx_btn1, 0, wxALL|wxALIGN_CENTER, 5);
	ctx_sizer->Add(ctx_btn2, 0, wxALL|wxALIGN_CENTER, 5);
	ctx_sizer->Add(ctx_btn3, 0, wxALL|wxALIGN_CENTER, 5);
	ctx_sizer->Add(context_spinctrl, 0, wxALL, 5);
	ctx_sizer->AddStretchSpacer();
	auto dummy_sizer = new wxBoxSizer(wxVERTICAL);
	dummy_sizer->Add(ctx_sizer, 1, wxEXPAND, 0);
	dummy_sizer->AddSpacer(20);
	context_box->SetSizer(dummy_sizer);

	// Create the constraints
	auto constraint_box = new wxStaticBox(panel, wxID_ANY, _("Constraints"));
	auto con_sizer = new wxBoxSizer(wxVERTICAL);
	con_sizer->Add(new ConstraintCtrl(constraint_box, wxRB_GROUP), 0, wxEXPAND);
	con_sizer->Add(new ConstraintCtrl(constraint_box, 0), 0, wxEXPAND, 0);
	con_sizer->AddSpacer(50);
	constraint_box->SetSizer(con_sizer);


	auto sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(context_box, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	sizer->Add(constraint_box, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
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

} // namespace phonometrica
