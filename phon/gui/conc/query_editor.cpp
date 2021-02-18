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

#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <phon/gui/conc/query_editor.hpp>
#include <phon/gui/dialog.hpp>
#include <phon/include/icons.hpp>
#include <phon/runtime.hpp>
#include <phon/application/macros.hpp>
#include <phon/application/project.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

int QueryEditor::id = 0;


QueryEditor::QueryEditor(wxWindow *parent, const wxString &title) :
	wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxMINIMIZE_BOX)
{
	properties_per_row = 4;
}

void QueryEditor::Prepare()
{
	// Since we can't call virtual functions in the constructor, we setup the UI after the object is set up.
	this->SetPosition(wxPoint(100, 70));

	auto main_window = new wxScrolledWindow(this);
	auto scrolled_sizer = new wxBoxSizer(wxVERTICAL);

#ifdef __WXGTK__
	const int search_prop = 0;
	const int prop = 0;
#elif defined(__WXMSW__)
	const int search_prop = 0;
	const int prop = 0;
#else
	const int search_prop = 2;
	const int prop = 1;
#endif

	scrolled_sizer->AddSpacer(5);
	scrolled_sizer->Add(MakeHeader(main_window), 0, wxEXPAND | wxALL, 10);
	scrolled_sizer->AddSpacer(5);
	scrolled_sizer->Add(MakeSearchPanel(main_window), search_prop, wxEXPAND | wxALL, 0);
	scrolled_sizer->Add(MakeFileSelector(main_window), prop, wxEXPAND | wxALL, 0);
	scrolled_sizer->Add(MakeProperties(main_window), prop, wxEXPAND | wxALL, 10);
#ifndef __WXMAC__
	scrolled_sizer->AddStretchSpacer();
#endif
	scrolled_sizer->Add(MakeButtons(main_window), 0, wxEXPAND | wxALL, 10);

	main_window->SetSizer(scrolled_sizer);
	auto main_sizer = new wxBoxSizer(wxVERTICAL);
	main_window->SetScrollRate(0, 5);

	main_sizer->Add(main_window, 1, wxEXPAND | wxALL, 0);
	SetSizer(main_sizer);
	prepared = true;
	// FIXME: I can't get the scrolled window to work properly on macOS: if we don't set the virtual size,
	//  the vertical scroll bar covers part of the OK button. With this it still shows but at least it
	//  doesn't overlap.
#ifdef __WXMAC__
	main_window->SetVirtualSize(500, 500);
#endif
}

wxBoxSizer *QueryEditor::MakeHeader(wxWindow *parent)
{
	auto sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(new wxStaticText(parent, wxID_ANY, _("Query name:")), 0, wxALIGN_CENTER, 0);
	wxString name = wxString::Format(_("Query %d"), GenerateId());
	name_ctrl = new wxTextCtrl(parent, wxID_ANY, name);
	name_ctrl->SetMinSize(wxSize(200, -1));
	sizer->AddSpacer(5);
	sizer->Add(name_ctrl);
	sizer->AddStretchSpacer();
#if __WXMAC__
	auto help_btn = new wxButton(parent, wxID_HELP);
#else
	auto help_btn = new wxButton(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	help_btn->SetBitmap(wxBITMAP_PNG_FROM_DATA(question));
	help_btn->SetMaxClientSize(wxSize(40, 100));
#endif
	help_btn->SetToolTip(_("Display help about metadata"));
	help_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &QueryEditor::OnOpenHelp, this);
	sizer->Add(help_btn, 0, 0, 0);

	name_ctrl->Bind(wxEVT_TEXT, &QueryEditor::OnQueryModified, this);

	return sizer;
}

wxBoxSizer *QueryEditor::MakeButtons(wxWindow *parent)
{
	auto sizer = new wxBoxSizer(wxHORIZONTAL);
	save_btn = new wxButton(parent, wxID_ANY, _("Save"));
	save_as_btn = new wxButton(parent, wxID_ANY, _("Save as..."));
	auto cancel_btn = new wxButton(parent, wxID_CANCEL, _("Cancel"));
	auto ok_btn = new wxButton(parent, wxID_OK, _("OK"));
	sizer->Add(save_btn);
	sizer->AddSpacer(5);
	sizer->Add(save_as_btn);
	sizer->AddStretchSpacer();
	sizer->Add(cancel_btn);
	sizer->AddSpacer(5);
	sizer->Add(ok_btn);

	save_btn->Disable();
	save_as_btn->Disable();

	ok_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &QueryEditor::OnOk, this);
	cancel_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &QueryEditor::OnCancel, this);
	save_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &QueryEditor::OnSave, this);
	save_as_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &QueryEditor::OnSaveAs, this);

	return sizer;
}

Handle<Concordance> QueryEditor::ExecuteQuery()
{
	if (!prepared) {
		throw error("Internal error: you must call Prepare() before executing a query");
	}
	ParseQuery();
	auto query = GetQuery();

	if (query->modified()) {
		Project::updated();
	}

	return query->execute();
}

void QueryEditor::OnOpenHelp(wxCommandEvent &)
{
	auto url = Settings::get_documentation_page("query.html");
	wxLaunchDefaultBrowser(url, wxBROWSER_NOBUSYCURSOR);
}

wxWindow *QueryEditor::MakeProperties(wxWindow *parent)
{
	auto property_box = new wxStaticBox(parent, wxID_ANY, _("File properties"));
	auto categories = Property::get_categories();

	if (categories.empty())
	{
		auto sizer = new wxBoxSizer(wxVERTICAL);
		auto label = new wxStaticText(property_box, wxID_ANY, _("The current project doesn't have any property."));
		label->SetForegroundColour(wxColor(100, 100, 100));
		auto font = label->GetFont();
		font.MakeBold();
		label->SetFont(font);
		sizer->AddSpacer(10);
		sizer->Add(label, 1, wxEXPAND|wxALL, 10);
		sizer->AddSpacer(30);
		property_box->SetSizer(sizer);

		return property_box;
	}

	auto sizer = new wxGridSizer(properties_per_row, 5, 5);

	for (auto &category : categories)
	{
		const std::type_info *type;

		if (Property::is_boolean(category))
		{
			type = &typeid(bool);
		}
		else if (Property::is_numeric(category))
		{
			type = &typeid(double);
		}
		else
		{
			type = &typeid(String);
		}

		auto prop = new PropertyCtrl(property_box, category, *type);
		properties.append(prop);
		sizer->Add(prop, 1, wxEXPAND);
		prop->modified.connect([this]() { EnableSaving(true); });
	}
	property_box->SetSizer(sizer);

	return property_box;
}

wxBoxSizer *QueryEditor::MakeFileSelector(wxWindow *parent)
{
	auto sizer = new wxBoxSizer(wxHORIZONTAL);
	auto file_box = new wxStaticBox(parent, wxID_ANY, _("Annotations"));
	auto file_sizer = new wxBoxSizer(wxVERTICAL);
	wxArrayString annotations, tooltips;
	for (auto &file : Project::get()->get_annotations()) {
		annotations.Add(file->label());
		tooltips.Add(file->path());
	}

	file_list = new CheckListBox(file_box, annotations, tooltips);
	file_list->SetMaxSize(FromDIP(wxSize(-1, 250)));
	auto old_font = file_list->GetFont();
	auto new_font = Settings::get_mono_font();
	new_font.SetPointSize(old_font.GetPointSize());
	file_list->SetFont(new_font);

#ifdef __WXMSW__
	file_sizer->AddSpacer(20);
#endif

	auto file_checkbox = new wxCheckBox(file_box, wxID_ANY, wxString());
	file_checkbox->SetToolTip(_("Select all files"));
	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	hsizer->AddStretchSpacer();
	hsizer->Add(file_checkbox);
	file_sizer->Add(hsizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);
	file_sizer->Add(file_list, 1, wxEXPAND|wxALL, 10);
	file_sizer->AddSpacer(10);
	file_box->SetSizer(file_sizer);
	file_box->SetToolTip(_("Select specific files or leave empty to search all the files"));
	sizer->Add(file_box, 1, wxEXPAND|wxLEFT, 10);
	sizer->AddSpacer(10);

	auto desc_box = new wxStaticBox(parent, wxID_ANY, _("File description"));
	auto desc_sizer = new wxBoxSizer(wxHORIZONTAL);
	desc_sizer->Add(new wxStaticText(desc_box, wxID_ANY, _("Description")), 0, wxALIGN_CENTER);
	desc_sizer->AddSpacer(5);
	wxArrayString operators;
	operators.Add(_("is exactly"));
	operators.Add(_("is not"));
	operators.Add(_("contains"));
	operators.Add(_("doesn't contain"));
	operators.Add(_("matches"));
	operators.Add(_("doesn't match"));
	desc_op_choice = new wxChoice(desc_box, wxID_ANY, wxDefaultPosition, wxDefaultSize, operators);
	desc_op_choice->SetSelection(2);
	desc_sizer->Add(desc_op_choice);
	desc_sizer->AddSpacer(5);
	desc_ctrl = new wxTextCtrl(desc_box, wxID_ANY);
	desc_sizer->Add(desc_ctrl, 1, wxEXPAND, 0);
	auto dummy_sizer = new wxBoxSizer(wxVERTICAL);
#ifdef __WXMSW__
    dummy_sizer->AddSpacer(20);
#endif
	dummy_sizer->Add(desc_sizer, 0, wxEXPAND|wxALL, 10);
	dummy_sizer->AddStretchSpacer();
	desc_box->SetSizer(dummy_sizer);
	sizer->Add(desc_box, 3, wxEXPAND|wxRIGHT, 10);

	desc_ctrl->Bind(wxEVT_TEXT, &QueryEditor::OnQueryModified, this);
	desc_op_choice->Bind(wxEVT_CHOICE, &QueryEditor::OnQueryModified, this);
	file_list->Bind(wxEVT_CHECKLISTBOX, &QueryEditor::OnQueryModified, this);
	file_checkbox->Bind(wxEVT_CHECKBOX, &QueryEditor::OnFileListChecked, this);

	return sizer;
}

void QueryEditor::OnOk(wxCommandEvent &)
{
	EndModal(wxID_OK);
}

void QueryEditor::OnCancel(wxCommandEvent &)
{
	EndModal(wxID_CANCEL);
}

void QueryEditor::OnSave(wxCommandEvent &e)
{
	ParseQuery();
	auto query = GetQuery();

	if (!query->has_path())
	{
		OnSaveAs(e);
	}
	if (query->modified()) {
		Project::updated();
	}
}

void QueryEditor::OnSaveAs(wxCommandEvent &)
{
	auto name = name_ctrl->GetValue();
	name.Replace(" ", "_");
	name.Append(PHON_EXT_QUERY);
	FileDialog dlg(this, _("Save query..."), name, "Phonometrica query (*.phon-query)|*.phon-query",
	               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

	if (dlg.ShowModal() == wxID_OK)
	{
		SaveQuery(dlg.GetPath());
	}
}

void QueryEditor::SaveQuery(const String &path)
{
	ParseQuery();
	auto query = GetQuery();
	query->set_path(path, true);
	query->save();

	if (query->parent() == nullptr)
	{
		Project::get()->add_query(std::move(query));
		Project::updated();
	}
}

void QueryEditor::EnableSaving(bool value)
{
	save_btn->Enable(value);
	save_as_btn->Enable(value);
}

void QueryEditor::OnQueryModified(wxCommandEvent &)
{
	EnableSaving(true);
}

void QueryEditor::OnFileListChecked(wxCommandEvent &e)
{
	file_list->CheckAll(e.IsChecked());
}

} // namespace phonometrica
