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
 * Created: 28/03/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/msgdlg.h>
#include <wx/stattext.h>
#include <phon/gui/pref/formant_settings.hpp>
#include <phon/gui/sizer.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

FormantSettings::FormantSettings(wxWindow *parent) : PreferencesDialog(parent, _("Formant settings..."))
{
	SetSize(wxSize(400, 440));
	auto panel = MakeGeneralPanel();
	AddPage(panel, _("General"));
}

void FormantSettings::DoReset()
{
	Settings::reset_formants();
}

bool FormantSettings::DoOk()
{
	bool ok;
	String category("formants");
	String text;

	text = nformant_ctrl->GetValue();
	auto nformant = text.to_int(&ok);
	if (!ok || nformant <= 0 || nformant > PHON_MAX_FORMANTS) {
		wxMessageBox(_("Invalid number of formants"), ("Invalid setting"), wxICON_ERROR);
		return false;
	}

	text = window_ctrl->GetValue();
	auto len = text.to_int(&ok);
	if (!ok || len <= 0) {
		wxMessageBox(_("Invalid window length"), ("Invalid setting"), wxICON_ERROR);
		return false;
	}

	text = npole_ctrl->GetValue();
	auto npole = text.to_int(&ok);
	if (!ok || npole <= nformant) {
		wxMessageBox(_("Invalid LPC order"), ("Invalid setting"), wxICON_ERROR);
		return false;
	}

	text = max_freq_ctrl->GetValue();
	auto fs = text.to_int(&ok);
	if (!ok || fs <= 0) {
		wxMessageBox(_("Invalid maximum frequency"), ("Invalid setting"), wxICON_ERROR);
		return false;
	}

	text = step_ctrl->GetValue();
	auto step = text.to_float(&ok);
	if (!ok || step <= 0.0) {
		wxMessageBox(_("Invalid time step"), ("Invalid setting"), wxICON_ERROR);
		return false;
	}

	Settings::set_value(category, "number_of_formants", nformant);
	Settings::set_value(category, "window_size", double(len)/1000);
	Settings::set_value(category, "lpc_order", npole);
	Settings::set_value(category, "max_frequency", fs);
	Settings::set_value(category, "time_step", step);

	return true;
}

wxPanel *FormantSettings::MakeGeneralPanel()
{
	auto panel = new wxPanel(m_book, wxID_ANY);
	auto sizer = new VBoxSizer;

	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Number of formants:")), 0, wxLEFT|wxTOP|wxRIGHT, 10);
	nformant_ctrl = new wxTextCtrl(panel, wxID_ANY);
	sizer->Add(nformant_ctrl, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);

	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Maximum frequency (Hz):")), 0, wxLEFT|wxTOP|wxRIGHT, 10);
	max_freq_ctrl = new wxTextCtrl(panel, wxID_ANY);
	sizer->Add(max_freq_ctrl, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);

	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Window length (ms):")), 0, wxLEFT|wxTOP|wxRIGHT, 10);
	window_ctrl = new wxTextCtrl(panel, wxID_ANY);
	sizer->Add(window_ctrl, 0, wxEXPAND|wxLEFT|wxTOP|wxRIGHT, 10);

	sizer->Add(new wxStaticText(panel, wxID_ANY, _("LPC order:")), 0, wxLEFT|wxTOP|wxRIGHT, 10);
	npole_ctrl = new wxTextCtrl(panel, wxID_ANY);
	sizer->Add(npole_ctrl, 0, wxEXPAND|wxLEFT|wxTOP|wxRIGHT, 10);

	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Time step (s):")), 0, wxLEFT|wxTOP|wxRIGHT, 10);
	step_ctrl = new wxTextCtrl(panel, wxID_ANY);
	sizer->Add(step_ctrl, 0, wxEXPAND|wxLEFT|wxTOP|wxRIGHT, 10);

	DisplayValues();
	panel->SetSizer(sizer);

	return panel;
}

void FormantSettings::DisplayValues()
{
	String category("formants");

	auto nformant = (int) Settings::get_number(category, "number_of_formants");
	nformant_ctrl->SetValue(wxString::Format("%d", nformant));

	auto len = Settings::get_number(category, "window_size");
	window_ctrl->SetValue(wxString::Format("%d", int(len * 1000)));

	auto npole = (int) Settings::get_number(category, "lpc_order");
	npole_ctrl->SetValue(wxString::Format("%d", npole));

	auto fs = (int) Settings::get_number(category, "max_frequency");
	max_freq_ctrl->SetValue(wxString::Format("%d", fs));

	auto step = Settings::get_number(category, "time_step");
	step_ctrl->SetValue(wxString::Format("%g", step));
}
} // namespace phonometrica
