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
 * Created: 20/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <phon/gui/pref/preferences_editor.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

PreferencesEditor::PreferencesEditor(wxWindow *parent) :
	PreferencesDialog(parent, _("Preferences"))
{
	AddPage(MakeGeneralPanel(), _("General"));
	AddPage(MakeAppearancePanel(), _("Appearance"));
}

wxPanel *PreferencesEditor::MakeGeneralPanel()
{
	auto panel = new wxPanel(m_book);

	// Main sizer.
	auto sizer = new wxBoxSizer(wxVERTICAL);
	m_autoload_checkbox = new wxCheckBox(panel, wxID_ANY, _("Load most recent project on startup"));
	m_autoload_checkbox->SetValue(Settings::get_boolean("autoload"));

	m_restore_views_checkbox = new wxCheckBox(panel, wxID_ANY, _("Restore open views on startup"));
	m_restore_views_checkbox->SetValue(Settings::get_boolean("restore_views"));
	m_restore_views_checkbox->Enable(Settings::get_boolean("autoload"));
	m_autoload_checkbox->Bind(wxEVT_CHECKBOX, [=](wxCommandEvent &e) { m_restore_views_checkbox->Enable(e.IsChecked()); });

	m_autosave_checkbox = new wxCheckBox(panel, wxID_ANY, _("Automatically save project on exit"));
	m_autosave_checkbox->SetValue(Settings::get_boolean("autosave"));

	m_autohints_checkbox = new wxCheckBox(panel, wxID_ANY, _("Activate syntax hints in script views by default"));
	m_autohints_checkbox->SetValue(Settings::get_boolean("autohints"));

	m_empty_conc_checkbox = new wxCheckBox(panel, wxID_ANY, _("Discard empty queries"));
	m_empty_conc_checkbox->SetValue(Settings::get_boolean("concordance", "discard_empty"));

	sizer->Add(m_autoload_checkbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM|wxTOP, 10);
	sizer->Add(m_restore_views_checkbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	sizer->Add(m_autosave_checkbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	sizer->Add(m_empty_conc_checkbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	sizer->Add(m_autohints_checkbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	sizer->AddStretchSpacer(1);
	sizer->AddSpacer(10);

	panel->SetSizer(sizer);

	return panel;
}

wxPanel *PreferencesEditor::MakeAppearancePanel()
{
	auto panel = new wxPanel(m_book);
	auto sizer = new wxBoxSizer(wxVERTICAL);
	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	hsizer->Add(new wxStaticText(panel, wxID_ANY, _("Monospaced (fixed-width) font:"), wxDefaultPosition, wxDefaultSize), 0, wxALIGN_CENTER, 0);
	hsizer->AddSpacer(10);
	m_font_picker = new wxFontPickerCtrl(panel, wxID_ANY, Settings::get_mono_font(), wxDefaultPosition, wxDefaultSize, 0);
	hsizer->Add(m_font_picker, 1, wxEXPAND, 0);
	sizer->Add(hsizer, 0, wxEXPAND|wxALL, 10);
	sizer->AddStretchSpacer();
	panel->SetSizer(sizer);

	return panel;
}

bool PreferencesEditor::DoOk()
{
	// General panel
	Settings::set_value("autoload", m_autoload_checkbox->GetValue());
	Settings::set_value("autosave", m_autosave_checkbox->GetValue());
	Settings::set_value("autohints", m_autohints_checkbox->GetValue());
	Settings::set_value("restore_views", m_restore_views_checkbox->GetValue());
	Settings::set_value("concordance", "discard_empty", m_empty_conc_checkbox->GetValue());

	// Appearance panel
	auto old_font = Settings::get_mono_font();
	auto new_font = m_font_picker->GetSelectedFont();
	Settings::set_mono_font(new_font);

	if (old_font != new_font) {
		wxMessageBox(_("The font change will take effect when script views and the project are reloaded."),
			   _("Font information"), wxICON_INFORMATION);
	}

	return true;
}

void PreferencesEditor::DoReset()
{
	Settings::reset();
}


} // namespace phonometrica