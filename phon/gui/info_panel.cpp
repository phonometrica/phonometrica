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
 * Created: 13/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <phon/gui/dialog.hpp>
#include <phon/gui/info_panel.hpp>
#include <phon/gui/csv_dialog.hpp>
#include <phon/application/project.hpp>
#include <phon/include/icons.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

static const int SIDE_PADDING = 10;


static inline wxFont MakeBold(wxFont font)
{
	font.SetWeight(wxFONTWEIGHT_BOLD);
	return font;
}


InfoPanel::InfoPanel(Runtime &rt, wxWindow *parent) :
		wxPanel(parent, -1), runtime(rt)
{
	auto sizer = new wxBoxSizer(wxVERTICAL);
	auto header = new wxStaticText(this, wxID_ANY, _("Information"), wxDefaultPosition, wxDefaultSize);
	header->SetForegroundColour(wxColor(75, 75, 75));
	auto font = header->GetFont();
	font.MakeBold();
	header->SetFont(font);

	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
#if __WXMAC__
    auto help_btn = new wxButton(this, wxID_HELP);
#else
    auto help_btn = new wxButton(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    help_btn->SetBitmap(wxBITMAP_PNG_FROM_DATA(question));
    help_btn->SetMaxClientSize(wxSize(40, 100));
#endif
	help_btn->SetToolTip(_("Display help about metadata"));
	help_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InfoPanel::OnOpenHelp, this);

	hsizer->Add(header, 1, wxALIGN_CENTER, 0);
	hsizer->AddStretchSpacer();
	hsizer->Add(help_btn);
	hsizer->AddSpacer(SIDE_PADDING);

	m_book = new wxSimplebook(this);
	SetupBook();
	sizer->AddSpacer(5);
	sizer->Add(hsizer, 0, wxEXPAND|wxLEFT, 7);
	sizer->Add(m_book, 1,wxEXPAND, 0);
	SetSizer(sizer);
}

void InfoPanel::SetupBook()
{
	SetEmptyPage();
	SetSingleFilePage();
	SetMultipleFilesPage();
	UpdateInformation();
}

void InfoPanel::SetEmptyPage()
{
	empty_page = new wxPanel(m_book);
	auto empty_text = new wxStaticText(empty_page, wxID_ANY, _("No metadata to display"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL|wxST_NO_AUTORESIZE);
	empty_text->SetForegroundColour(wxColor(175, 175, 175)); // grey
	auto font = empty_text->GetFont();
	font.MakeBold();
	empty_text->SetFont(font);

	auto sizer = new wxBoxSizer(wxVERTICAL);
	sizer->AddStretchSpacer(2);
	sizer->Add(empty_text, 0, wxEXPAND, 0);
	sizer->AddStretchSpacer(3);
	empty_page->SetSizer(sizer);

	m_book->AddPage(empty_page, _("File information"));
}

void InfoPanel::SetSingleFilePage()
{
	single_page = new wxPanel(m_book);
	auto sizer = new wxBoxSizer(wxVERTICAL);
	single_page->SetSizer(sizer);
	m_book->AddPage(single_page, _("File information"));
}

void InfoPanel::SetMultipleFilesPage()
{
	multiple_page = new wxPanel(m_book);
	auto sizer = new wxBoxSizer(wxVERTICAL);
	multiple_page->SetSizer(sizer);
	m_book->AddPage(multiple_page, _("File information"));
}

void InfoPanel::OnSetFileSelection(VFileList files)
{
	m_files = std::move(files);
	UpdateInformation();
}

void InfoPanel::UpdateInformation()
{
	auto id = std::min<intptr_t>(m_files.size(), 2);
	has_unsaved_property = false;

	if (id == 1)
	{
		ClearPanel(single_page);
		DisplaySingleFile();
	}
	else if (id == 2)
	{
		ClearPanel(multiple_page);
		DisplayMultipleFiles();
	}

	m_book->SetSelection(id);
	m_book->GetCurrentPage()->Layout();
}

void InfoPanel::DisplaySingleFile()
{
	auto file = m_files.first().get();

	single_page->GetSizer()->AddSpacer(5);
	AddSectionHeading(single_page, _("File name:"), false);
	AddLabel(single_page, file->label(), file->path());

	if (file->is_annotation())
	{
		auto annot = dynamic_cast<Annotation*>(file);
		auto sound = annot->sound();

		AddSectionHeading(single_page, _("Sound file:"), false);
		auto label = sound ? wxString(sound->label()) : _("None");
		auto path = sound ? wxString(sound->path()) : wxString();
		AddSoundLabel(single_page, label, path);
	}

	AddProperties(single_page, false);
	AddDescription(file->description());
	AddMetadataButtons(single_page);
}

void InfoPanel::DisplayMultipleFiles()
{
	AddSectionHeading(multiple_page, _("Files:"), false);

	for (auto &f: m_files) {
		AddLabel(multiple_page, f->label());
	}

	AddProperties(multiple_page, true);
	multiple_page->GetSizer()->AddStretchSpacer();
	AddMetadataButtons(multiple_page);
}

void InfoPanel::AddDescription(const wxString &desc)
{
	ctrl_desc = new wxRichTextCtrl(single_page, -1, desc, wxDefaultPosition, wxDefaultSize, wxRE_MULTILINE);
	AddSectionHeading(single_page, "Description:", false);
	single_page->GetSizer()->AddSpacer(5);
	single_page->GetSizer()->Add(ctrl_desc, 1, wxEXPAND|wxRIGHT|wxLEFT, SIDE_PADDING);
	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	auto btn = new wxButton(single_page, wxID_ANY, _("Save description"));
	btn->SetToolTip(_("Set file description"));
	btn->Enable(false);
	hsizer->Add(btn, 1, wxEXPAND, 0);
	single_page->GetSizer()->Add(hsizer, 0, wxEXPAND|wxALL, SIDE_PADDING);
	ctrl_desc->Bind(wxEVT_TEXT, &InfoPanel::OnDescriptionEdited, this);
	btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InfoPanel::OnSaveDescription, this);
	save_desc_btn = btn;
}

void InfoPanel::ClearPanel(wxPanel *panel)
{
	if (prop_ctrl) {
		prop_ctrl->Unbind(wxEVT_SIZE, &InfoPanel::OnResizeProperties, this);
	}

	wxWindowList children = panel->GetChildren();

	for (size_t i = 0; i < children.GetCount(); ++i) {
		delete children[i];
	}
	panel->GetSizer()->Clear();
	prop_ctrl = nullptr;
}

void InfoPanel::AddSectionHeading(wxPanel *panel, const wxString &header, bool add_space)
{
	auto sizer = panel->GetSizer();
	auto label = new wxStaticText(panel, wxID_ANY, header);
	label->SetFont(MakeBold(label->GetFont()));

	if (add_space)
	{
		sizer->AddStretchSpacer(1);
	}
	sizer->AddSpacer(10);
	sizer->Add(label, 0, wxEXPAND|wxLEFT, SIDE_PADDING);
	sizer->AddSpacer(5);
}

void InfoPanel::AddLabel(wxPanel *panel, const wxString &label, const wxString &tooltip)
{
	auto sizer = panel->GetSizer();
	auto txt = new wxStaticText(panel, wxID_ANY, label);
	if (!tooltip.IsEmpty()) {
		txt->SetToolTip(tooltip);
	}
	sizer->Add(txt, 0, wxEXPAND|wxLEFT, SIDE_PADDING);
}

void InfoPanel::AddSoundLabel(wxPanel *panel, const wxString &label, const wxString &path)
{
	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	auto txt = new wxStaticText(panel, -1, label);
	txt->SetToolTip(path);
	// Note: wxALIGN_CENTER gets the static text centered vertically in the horizontal sizer
	hsizer->Add(txt, 1, wxALIGN_CENTER|wxLEFT, SIDE_PADDING);
	auto btn = new wxButton(panel, wxID_ANY, _("Bind..."));
	btn->SetToolTip(_("Bind annotation to sound file"));
	hsizer->Add(btn);
	panel->GetSizer()->Add(hsizer, 0, wxEXPAND|wxRIGHT, SIDE_PADDING);

	btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InfoPanel::OnBindSound, this);
}

void InfoPanel::AddProperties(wxPanel *panel, bool shared)
{
	auto label = shared ? _("Shared properties:") : _("Properties:");
	AddSectionHeading(panel, label, false);


	prop_ctrl = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_VRULES);
	SetMinClientSize(FromDIP(wxSize(50, 150)));
	SetProperties(wxString());
	prop_ctrl->Bind(wxEVT_SIZE, &InfoPanel::OnResizeProperties, this);
	prop_ctrl->Bind(wxEVT_LIST_ITEM_SELECTED, &InfoPanel::OnPropertySelected, this);

	panel->GetSizer()->Add(prop_ctrl, 0, wxEXPAND|wxALL, SIDE_PADDING);
	AddPropertyButtons(panel);
}

void InfoPanel::AddPropertyButtons(wxPanel *panel)
{
#ifdef __WXMAC__
	auto add_btn = new wxButton(panel, wxID_ANY, "+");
	auto rm_btn = new wxButton(panel, wxID_ANY, "-");
#else
	auto add_btn = new wxButton(panel, wxID_ANY, wxEmptyString);
	auto rm_btn = new wxButton(panel, wxID_ANY, wxEmptyString);
	add_btn->SetBitmap(wxBITMAP_PNG_FROM_DATA(plus), wxTOP);
	rm_btn->SetBitmap(wxBITMAP_PNG_FROM_DATA(minus));
#endif
	rm_btn->Enable(false);
	add_btn->SetMaxClientSize(wxSize(40, -1));
	rm_btn->SetMaxClientSize(wxSize(40, -1));
	add_btn->SetToolTip(_("Add property"));
	rm_btn->SetToolTip(_("Remove property"));
	prop_rm_btn = rm_btn;

	add_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InfoPanel::OnAddProperty, this);
	rm_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InfoPanel::OnRemoveProperty, this);

	// Merge the type_choice static text with the +/- buttons to save vertical space
	auto type_sizer = new wxBoxSizer(wxHORIZONTAL);
	type_sizer->Add(new wxStaticText(panel, wxID_ANY, _("Edit type:")), 0, wxALIGN_BOTTOM, 0);
	type_sizer->AddStretchSpacer();
	type_sizer->Add(add_btn, 0, 0, 0);
	type_sizer->AddSpacer(5);
	type_sizer->Add(rm_btn, 0, 0, 0);

	auto sizer = panel->GetSizer();
	sizer->Add(type_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT, SIDE_PADDING);
	sizer->AddSpacer(5);

	wxArrayString types;
	types.Add("Text");
	types.Add("Number");
	types.Add("Boolean");
	type_choice = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, types);
	type_choice->SetToolTip(_("Set property type"));
	sizer->Add(type_choice, 0, wxEXPAND | wxALL, SIDE_PADDING);
	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Edit category:")),  0, wxEXPAND|wxEXPAND|wxLEFT|wxRIGHT, SIDE_PADDING);
	category_combo = new wxComboBox(panel, wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, wxArrayString(), wxTE_PROCESS_ENTER);
	category_combo->SetToolTip(_("Set property category"));
	sizer->Add(category_combo, 0, wxEXPAND|wxALL, SIDE_PADDING);
	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Edit value:")),  0, wxEXPAND|wxLEFT|wxRIGHT, SIDE_PADDING);
	value_combo = new wxComboBox(panel, wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize);
	value_combo->SetToolTip(_("Set property value"));

	category_combo->Bind(wxEVT_KEY_DOWN, &InfoPanel::OnCategoryComboKeyPressed, this);
	value_combo->Bind(wxEVT_SET_FOCUS, &InfoPanel::OnValueComboActivated, this);
	value_combo->Bind(wxEVT_KEY_DOWN, &InfoPanel::OnValueComboKeyPressed, this);

	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	validate_btn = new wxButton(panel, wxID_ANY, _("Validate"));
	validate_btn->SetToolTip(_("Validate property"));
	clear_btn = new wxButton(panel, wxID_ANY, _("Clear"));
	clear_btn->SetToolTip(_("Clear property"));
	sizer->Add(value_combo, 0, wxEXPAND | wxALL, SIDE_PADDING);

	hsizer->Add(validate_btn, 0, wxEXPAND, 0);
	hsizer->AddSpacer(5);
	hsizer->Add(clear_btn, 0, wxEXPAND, 0);
	hsizer->AddStretchSpacer();
	sizer->Add(hsizer, 0, wxEXPAND|wxALL, SIDE_PADDING);

	EnablePropertyEditing(false);

	type_choice->Bind(wxEVT_CHOICE, &InfoPanel::OnTypeChosen, this);
	category_combo->Bind(wxEVT_TEXT_ENTER, &InfoPanel::OnCategoryPressEnter, this);
	category_combo->Bind(wxEVT_COMBOBOX, &InfoPanel::OnCategoryChanged, this);

	validate_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InfoPanel::OnValidateProperty, this);
	clear_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InfoPanel::OnClearProperty, this);
}

void InfoPanel::OnSaveDescription(wxCommandEvent &)
{
	auto file = m_files.front().get();
	file->set_description(ctrl_desc->GetValue());
	save_desc_btn->Enable(false);
	Project::updated();
}

void InfoPanel::OnBindSound(wxCommandEvent &)
{
	FileDialog dlg(this, _("Bind annotation to sound file..."), "", "Sound file (*.*)|*.*",
	                 wxFD_OPEN|wxFD_FILE_MUST_EXIST);

	if (dlg.ShowModal() == wxID_OK)
	{
		auto project = Project::get();
		String path = dlg.GetPath();
		auto ext = filesystem::ext(path, true, true);
		auto &formats = Sound::supported_sound_formats();

		if (! formats.contains(ext))
		{
			auto msg = utils::format("'%' is not a supported sound format", ext);
			wxMessageBox(msg, _("Invalid sound file"), wxICON_ERROR);
			return;
		}

		auto annot = downcast<Annotation>(m_files.first());
		project->import_file(path);
		auto sound = downcast<Sound>(project->get(path));

		if (annot && sound)
		{
			annot->set_sound(sound);
			UpdateInformation();
		}
		else
		{
			wxMessageBox(_("Could not bind annotation and sound file"), _("Invalid sound file"), wxICON_ERROR);
		}
	}
}

void InfoPanel::OnAddProperty(wxCommandEvent &)
{
	EnablePropertyEditing(true);
	auto new_row = prop_ctrl->InsertItem(prop_ctrl->GetItemCount(), _("undefined"));
	prop_ctrl->SetItem(new_row, 1, _("undefined"));
	prop_ctrl->SetItem(new_row, 2, _("undefined"));
	auto font = prop_ctrl->GetItemFont(new_row);
	font.MakeItalic();
	prop_ctrl->SetItemFont(new_row, font);
	prop_ctrl->SetItemTextColour(new_row, *wxRED);

	long item = -1;
	while (true)
	{
		item = prop_ctrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1) {
			break;
		}

		prop_ctrl->SetItemState(item, 0, wxLIST_STATE_SELECTED);
	}
	prop_ctrl->SetItemState(new_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	has_unsaved_property = true;
	prop_rm_btn->Enable(true);
}

void InfoPanel::OnRemoveProperty(wxCommandEvent &e)
{
	long row = prop_ctrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (row == -1) {
		return;
	}
	String category = prop_ctrl->GetItemText(row, 1);
	for (auto &file : m_files)
	{
		file->remove_property(category);
	}
	prop_ctrl->DeleteItem(row);
	prop_rm_btn->Enable(false);
	has_unsaved_property = false;
	EnablePropertyEditing(false);
	Project::get()->metadata_updated();
}

void InfoPanel::OnImportMetadata(wxCommandEvent &)
{
	ImportMetadata();
}

void InfoPanel::OnPropertySelected(wxListEvent &)
{
	if (has_unsaved_property)
	{
		auto reply = ask_question(_("Discard unsaved property?"), _("Edit properties"));
		if (reply != wxYES)
		{
			return;
		}
		prop_ctrl->DeleteItem(prop_ctrl->GetItemCount() - 1);
		has_unsaved_property = false;
	}
	prop_rm_btn->Enable(true);
}

void InfoPanel::OnDescriptionEdited(wxCommandEvent &)
{
	save_desc_btn->Enable(true);
}

void InfoPanel::ImportMetadata()
{
	CsvDialog dlg(this, _("Import metadata..."), true);

	if (dlg.ShowModal() == wxID_OK)
	{
		auto path = dlg.GetPath();
		auto sep = dlg.GetSeparator();
		Project::get()->import_metadata(path, sep);
		UpdateInformation();
		wxMessageBox(_("Metadata successfully imported!"), _("Success"), wxICON_INFORMATION);
	}
}

void InfoPanel::OnExportMetadata(wxCommandEvent &)
{
	ExportMetadata();
}

void InfoPanel::ExportMetadata()
{
	CsvDialog dlg(this, _("Export metadata..."), false);

	if (dlg.ShowModal() == wxID_CANCEL) {
		return;
	}
	Project::get()->export_metadata(dlg.GetPath());
	wxMessageBox(_("Metadata sucessfully exported!"), _("Success"), wxICON_INFORMATION);
}

void InfoPanel::AddMetadataButtons(wxPanel *panel)
{
	auto sizer = panel->GetSizer();
	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	auto import_btn = new wxButton(panel, wxID_ANY, _("Import metadata..."));
	auto export_btn = new wxButton(panel, wxID_ANY, _("Export metadata..."));
	import_btn->SetToolTip(_("Import metadata from CSV file"));
	export_btn->SetToolTip(_("Export project metadata to CSV file"));
	hsizer->Add(import_btn, 1, wxEXPAND, 0);
	hsizer->AddSpacer(5);
	hsizer->Add(export_btn, 1, wxEXPAND, 0);
	sizer->Add(hsizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, SIDE_PADDING);

	import_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InfoPanel::OnImportMetadata, this);
	export_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InfoPanel::OnExportMetadata, this);
}

void InfoPanel::OnResizeProperties(wxSizeEvent &e)
{
	// We need to check the number of columns because a resize event might happen when we clear
	// the properties to refresh them.
	if (prop_ctrl->GetColumnCount() == 3)
	{
		ResizeProperties();
	}
	e.Skip();
}

void InfoPanel::ResizeProperties()
{
	int w = prop_ctrl->GetClientSize().GetWidth();
	prop_ctrl->SetColumnWidth(0, w/3);
	prop_ctrl->SetColumnWidth(1, w/3);
	prop_ctrl->SetColumnWidth(2, w/3);
}

void InfoPanel::EnablePropertyEditing(bool value)
{
	if (!value)
	{
		type_choice->SetSelection(wxNOT_FOUND);
		category_combo->Clear();
		value_combo->Clear();
	}
	type_choice->Enable(value);
	category_combo->Enable(value);
	value_combo->Enable(value);
	validate_btn->Enable(value);
	clear_btn->Enable(value);
}

void InfoPanel::OnOpenHelp(wxCommandEvent &)
{
	auto url = Settings::get_documentation_page("intro/start.html");
	wxLaunchDefaultBrowser(url, wxBROWSER_NOBUSYCURSOR);
}

void InfoPanel::OnTypeChosen(wxCommandEvent &)
{
	std::set<String> categories;
	category_combo->Clear();
	value_combo->Clear();

	if (type_choice->GetSelection() == 0) // Text
	{
		categories = Property::get_categories_by_type(typeid(String));
	}
	else if (type_choice->GetSelection() == 1) // Number
	{
		categories = Property::get_categories_by_type(typeid(double));
	}
	else // Boolean
	{
		categories = Property::get_categories_by_type(typeid(bool));
		value_combo->Append("true");
		value_combo->Append("false");
	}

	for (auto &cat : categories) {
		category_combo->Append(cat);
	}
	category_combo->SetFocus();
}

void InfoPanel::OnCategoryPressEnter(wxCommandEvent &)
{
	value_combo->SetFocus();
}

void InfoPanel::UpdateValues()
{
	value_combo->Clear();
	if (type_choice->GetSelection() == 0) // Text
	{
		String category = category_combo->GetValue();
		for (auto &cat : Property::get_values(category)) {
			value_combo->Append(cat);
		}
	}
	else if (type_choice->GetSelection() == 2) // Boolean
	{
		value_combo->Append("true");
		value_combo->Append("false");
	}
}

void InfoPanel::OnCategoryChanged(wxCommandEvent &)
{
	UpdateValues();
}

void InfoPanel::OnValidateProperty(wxCommandEvent &)
{
	ValidateProperty();
}

void InfoPanel::ValidateProperty()
{
	String type;
	switch (type_choice->GetSelection())
	{
		case 0:
			type = "Text";
			break;
		case 1:
			type = "Number";
			break;
		default:
			type = "Boolean";
	}
	String category = category_combo->GetValue();
	String value = value_combo->GetValue();
	Property prop;

	try
	{
		if (category.empty()) {
			throw error("Empty category");
		}
		prop = Property::parse(type, category, value);
	}
	catch (std::exception &e)
	{
		wxMessageBox(e.what(), _("Invalid property"), wxICON_ERROR);
		return;
	}

	for (auto &file : m_files) {
		file->add_property(prop);
	}
	EnablePropertyEditing(false);
	prop_ctrl->ClearAll();
	has_unsaved_property = false;
	SetProperties(category);
	Project::get()->metadata_updated();
}

void InfoPanel::OnClearProperty(wxCommandEvent &)
{
	category_combo->SetValue(wxString());
	value_combo->SetValue(wxString());
}

void InfoPanel::SetProperties(const wxString &selected)
{
	prop_ctrl->AppendColumn(_("Type"));
	prop_ctrl->AppendColumn(_("Key"));
	prop_ctrl->AppendColumn(_("Value"));

	for (auto &prop : Project::get_shared_properties(m_files))
	{
		long row = prop_ctrl->InsertItem(prop_ctrl->GetItemCount(), prop.type_name());
		prop_ctrl->SetItem(row, 1, prop.category());
		prop_ctrl->SetItem(row, 2, prop.value());

	}

	for (int i = 0; i < prop_ctrl->GetItemCount(); i++)
	{
		if (prop_ctrl->GetItemText(i, 1) == selected)
		{
			prop_ctrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
			break;
		}
	}

	ResizeProperties();
}

void InfoPanel::OnValueComboActivated(wxFocusEvent &)
{
	UpdateValues();
}

void InfoPanel::OnCategoryComboKeyPressed(wxKeyEvent &e)
{
	if (e.GetKeyCode() == WXK_TAB)
	{
		UpdateValues();
		value_combo->SetFocus();
	}
	else
	{
		e.Skip();
	}
}

void InfoPanel::OnValueComboKeyPressed(wxKeyEvent &e)
{
	if (e.GetKeyCode() == WXK_RETURN || e.GetKeyCode() == WXK_NUMPAD_ENTER)
	{
		ValidateProperty();
	}
	else
	{
		e.Skip();
	}
}


} // namespace phonometrica