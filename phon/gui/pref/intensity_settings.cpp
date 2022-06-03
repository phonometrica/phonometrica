/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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
 * Created: 25/05/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <phon/gui/sizer.hpp>
#include <phon/gui/pref/intensity_settings.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

IntensitySettings::IntensitySettings(wxWindow *parent) :
		PreferencesDialog(parent, _("Intensity settings..."))
{
	SetSize(wxSize(400, 350));
	auto panel = MakeGeneralPanel();
	AddPage(panel, _("General"));
}

wxPanel *IntensitySettings::MakeGeneralPanel()
{
	auto panel = new wxPanel(m_book, wxID_ANY);
	auto sizer = new VBoxSizer;

	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Minimum intensity (dB):")), 0, wxLEFT|wxTOP|wxRIGHT, 10);
	min_ctrl = new wxTextCtrl(panel, wxID_ANY);
	sizer->Add(min_ctrl, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);

	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Maximum intensity (db):")), 0, wxLEFT|wxTOP|wxRIGHT, 10);
	max_ctrl = new wxTextCtrl(panel, wxID_ANY);
	sizer->Add(max_ctrl, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);

	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Time step (s):")), 0, wxLEFT|wxTOP|wxRIGHT, 10);
	step_ctrl = new wxTextCtrl(panel, wxID_ANY);
	sizer->Add(step_ctrl, 0, wxEXPAND|wxLEFT|wxTOP|wxRIGHT, 10);

	DisplayValues();
	panel->SetSizer(sizer);

	return panel;
}

void IntensitySettings::DoReset()
{
	Settings::reset_intensity();
}

bool IntensitySettings::DoOk()
{
	bool ok;
	String category("intensity");
	String text;

	text = min_ctrl->GetValue();
	auto min_db = text.to_int(&ok);
	if (!ok || min_db < 0) {
		wxMessageBox(_("Invalid minimum intensity"), ("Invalid setting"), wxICON_ERROR);
		return false;
	}

	text = max_ctrl->GetValue();
	auto max_db = text.to_int(&ok);
	if (!ok || max_db <= min_db) {
		wxMessageBox(_("Invalid maximum intensity"), ("Invalid setting"), wxICON_ERROR);
		return false;
	}

	text = step_ctrl->GetValue();
	auto step = text.to_float(&ok);
	if (!ok || step <= 0.0) {
		wxMessageBox(_("Invalid time step"), ("Invalid setting"), wxICON_ERROR);
		return false;
	}

	Settings::set_value(category, "minimum_intensity", min_db);
	Settings::set_value(category, "maximum_intensity", max_db);
	Settings::set_value(category, "time_step", step);

	return true;
}

void IntensitySettings::DisplayValues()
{
	String category("intensity");

	auto max_db = (int) Settings::get_number(category, "maximum_intensity");
	max_ctrl->SetValue(wxString::Format("%d", max_db));

	auto min_db = (int) Settings::get_number(category, "minimum_intensity");
	min_ctrl->SetValue(wxString::Format("%d", min_db));

	auto step = Settings::get_number(category, "time_step");
	step_ctrl->SetValue(wxString::Format("%g", step));
}
} // namespace phonometrica
