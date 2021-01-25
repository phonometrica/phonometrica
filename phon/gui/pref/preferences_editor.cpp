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

#include <wx/wx.h>
#include <phon/gui/pref/preferences_editor.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

PreferencesEditor::PreferencesEditor(wxWindow *parent) :
	PreferencesDialog(parent, _("Preferences"))
{
	AddPage(MakeGeneralPanel(), _("General"));
	AddPage(MakeSoundPanel(), _("Sound"));
}

wxPanel *PreferencesEditor::MakeGeneralPanel()
{
	auto panel = new wxPanel(m_book);

	m_match_window_ctrl = new wxSpinCtrl(panel, wxID_ANY);
	m_match_window_ctrl->SetRange(1, 100);
	m_match_window_ctrl->SetValue((int)Settings::get_number("match_window_length"));

	// Main sizer.
	auto sizer = new wxBoxSizer(wxVERTICAL);
	auto match_sizer = new wxBoxSizer(wxHORIZONTAL);
	match_sizer->Add(new wxStaticText(panel, wxID_ANY, _("Match context length:"), wxDefaultPosition, wxDefaultSize, wxALIGN_BOTTOM), 0, wxEXPAND, 0);
	match_sizer->AddSpacer(10);
	match_sizer->Add(m_match_window_ctrl, 0, wxEXPAND, 0);
	sizer->AddSpacer(10);
	sizer->Add(match_sizer, 0, wxEXPAND|wxALL, 10);
	m_autoload_checkbox = new wxCheckBox(panel, wxID_ANY, _("Load most recent project on startup"));
	m_autoload_checkbox->SetValue(Settings::get_boolean("autoload"));
	sizer->Add(m_autoload_checkbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	m_autosave_checkbox = new wxCheckBox(panel, wxID_ANY, _("Automatically save project on exit"));
	m_autosave_checkbox->SetValue(Settings::get_boolean("autosave"));
	sizer->Add(m_autosave_checkbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);
	sizer->AddStretchSpacer(1);
	sizer->AddSpacer(10);

	panel->SetSizer(sizer);

	return panel;
}

wxPanel *PreferencesEditor::MakeSoundPanel()
{
	auto panel = new wxPanel(m_book);

	// Main sizer.
	auto sizer = new wxBoxSizer(wxVERTICAL);


	panel->SetSizer(sizer);


	return panel;

}

void PreferencesEditor::DoOk()
{
	// General panel
	Settings::set_value("match_window_length", (intptr_t)m_match_window_ctrl->GetValue());
	Settings::set_value("autoload", m_autoload_checkbox->GetValue());
	Settings::set_value("autosave", m_autosave_checkbox->GetValue());
}

void PreferencesEditor::DoReset()
{

}


} // namespace phonometrica