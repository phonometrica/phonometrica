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
#include <wx/button.h>
#include <wx/filedlg.h>
#include <phon/gui/info_panel.hpp>
#include <phon/gui/csv_dialog.hpp>
#include <phon/application/project.hpp>
#include <phon/include/icons.hpp>
#include <phon/utils/file_system.hpp>

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
	m_book = new wxSimplebook(this);
	SetupBook();
	sizer->Add(header, 0, wxTOP|wxLEFT, 7);
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
	wxWindowList children = panel->GetChildren();

	for (size_t i = 0; i < children.GetCount(); ++i) {
		delete children[i];
	}
	panel->GetSizer()->Clear();
	grid = nullptr;
}

void InfoPanel::AddSectionHeading(wxPanel *panel, const wxString &header, bool add_space)
{
	auto sizer = panel->GetSizer();
	auto label = new wxStaticText(panel, -1, header);
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
	auto txt = new wxStaticText(panel, -1, label);
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
	hsizer->Add(btn, 0, wxRIGHT, SIDE_PADDING);
	panel->GetSizer()->Add(hsizer, 0, wxEXPAND, 0);

	btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InfoPanel::OnBindSound, this);
}

void InfoPanel::AddProperties(wxPanel *panel, bool shared)
{
	auto label = shared ? _("Shared properties:") : _("Properties:");
	AddSectionHeading(panel, label, false);
	wxSize size(50, 150);

	grid = new PropertyGrid(panel);
	grid->SetMinClientSize(FromDIP(size));
	grid->Bind(wxEVT_GRID_CELL_CHANGED, &InfoPanel::OnCellChanged, this);
	grid->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &InfoPanel::OnPropertySelected, this);
	grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &InfoPanel::OnChangePropertyValue, this);

	auto properties = shared ? Project::get_shared_properties(m_files) : m_files.front()->properties();
	grid->AppendProperties(properties);

	panel->GetSizer()->Add(grid, 0, wxEXPAND|wxALL, SIDE_PADDING);
	AddPropertyButtons(panel);
}

void InfoPanel::AddPropertyButtons(wxPanel *panel)
{
	auto add_btn = new wxButton(panel, wxID_ANY, wxEmptyString);
	auto rm_btn = new wxButton(panel, wxID_ANY, wxEmptyString);
	add_btn->SetBitmap(wxBITMAP_PNG_FROM_DATA(plus));
	rm_btn->SetBitmap(wxBITMAP_PNG_FROM_DATA(minus));
	rm_btn->Enable(false);
	add_btn->SetMaxClientSize(wxSize(40, -1));
	rm_btn->SetMaxClientSize(wxSize(40, -1));
	add_btn->SetToolTip(_("Add property"));
	rm_btn->SetToolTip(_("Remove property"));
	prop_rm_btn = rm_btn;

	add_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InfoPanel::OnAddProperty, this);
	rm_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InfoPanel::OnRemoveProperty, this);

	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	hsizer->Add(add_btn, 0, 0, 0);
	hsizer->AddSpacer(5);
	hsizer->Add(rm_btn, 0, 0, 0);
	hsizer->AddStretchSpacer();

	auto sizer = panel->GetSizer();
	sizer->Add(hsizer, 0, wxEXPAND|wxLEFT|wxRIGHT, SIDE_PADDING);
	sizer->AddSpacer(5);
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
	wxFileDialog dlg(this, _("Bind annotation to sound file..."), "", "", "Sound file (*.*)|*.*",
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
	int row = grid->GetNumberRows();
	grid->AppendRows(1);

	wxArrayString types;
	types.Add("Text");
	types.Add("Number");
	types.Add("Boolean");
	auto ed = new wxGridCellChoiceEditor(types);
	grid->SetCellEditor(row, 0, ed);
	grid->SetGridCursor(row, 0);
	grid->SetEditingMode(row, true);
	grid->EnableCellEditControl(true);
	prop_rm_btn->Enable(true);
}

void InfoPanel::OnRemoveProperty(wxCommandEvent &e)
{
	auto indices = grid->GetSelectedRows();
	std::reverse(indices.begin(), indices.end());

	for (int i : indices)
	{
		String category = grid->GetCellValue(i, 0);
		for (auto &file : m_files)
		{
			file->remove_property(category);
		}
		grid->DeleteRows(i, 1);
	}
	prop_rm_btn->Enable(false);
	Project::get()->metadata_updated();
}

void InfoPanel::OnImportMetadata(wxCommandEvent &)
{
	ImportMetadata();
}

void InfoPanel::OnPropertySelected(wxGridEvent &e)
{
	grid->SelectRow(e.GetRow());
	prop_rm_btn->Enable(true);
}

void InfoPanel::OnChangePropertyValue(wxGridEvent &e)
{
	grid->SetGridCursor(e.GetRow(), e.GetCol());
	grid->EnableCellEditControl();
}

void InfoPanel::OnCellChanged(wxGridEvent &event)
{
	auto row = event.GetRow();
	String category = grid->GetCellValue(row, 1);
	String value = grid->GetCellValue(row, 2);
	Property prop;

	// Update property when we have a valid category and value.
	if (category.empty() || value.empty()) return;

	try
	{
		// Check whether the category already exists
		int nrow = grid->GetNumberRows();

		for (int i = 0; i < nrow; i++)
		{
			if (i == row) continue;
			String candidate = grid->GetCellValue(i, 1);
			if (candidate == category)
			{
				wxMessageBox(_("This category already exists"), _("Invalid property"), wxICON_ERROR);
				grid->SetCellValue(row, 1, wxString());
				event.Veto();
				return;
			}
		}

		String type = grid->GetCellValue(row, 0);

		if (value == "true" || value == "false")
		{
			if (type != "Boolean")
			{
				wxMessageBox(_("'true' and 'false' are only valid for Boolean properties"), _("Invalid property"), wxICON_ERROR);
				event.Veto();
				return;
			}
			prop = Property(category, (value == "true"));
		}
		else if (type == "Boolean")
		{
			wxMessageBox(_("Boolean value can only be \"true\" or \"false\""), _("Invalid property"), wxICON_ERROR);
			event.Veto();
			return;
		}
		else if (type == "Number")
		{
			bool ok;
			double result = value.to_float(&ok);
			if (!ok) {
				wxMessageBox(_("Invalid numeric value"), _("Invalid property"), wxICON_ERROR);
				event.Veto();
				return;
			}
			prop = Property(category, result);
		}
		else
		{
			prop = Property(category, value);
		}

		// All good
		for (auto &file : m_files) {
			file->add_property(prop);
		}
		grid->SetEditingMode(row, false);
		Project::get()->metadata_updated();
	}
	catch (std::exception &e)
	{
		wxMessageBox(e.what(), _("Invalid property"), wxICON_ERROR);
		grid->SetGridCursor(row, 0);
		grid->EnableCellEditControl();
		event.Veto();
	}
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


} // namespace phonometrica