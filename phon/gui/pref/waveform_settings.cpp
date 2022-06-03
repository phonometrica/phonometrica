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
 * Created: 21/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/msgdlg.h>
#include <phon/gui/pref/waveform_settings.hpp>
#include <phon/gui/sizer.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

WaveformSettings::WaveformSettings(wxWindow *parent) :
	PreferencesDialog(parent, _("Waveform settings..."))
{
	auto panel = new wxPanel(m_book);
	auto sizer = new VBoxSizer;
	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Scaling:")), 0, wxEXPAND|wxALL, 10);
	wxArrayString choices;
	choices.Add(_("Global magnitude"));
	choices.Add(_("Local magnitude"));
	choices.Add(_("Fixed magnitude"));
	scaling_choice = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
	sizer->Add(scaling_choice, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	scaling_ctrl = new wxTextCtrl(panel, wxID_ANY);
	sizer->Add(scaling_ctrl, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	panel->SetSizer(sizer);
	scaling_choice->Bind(wxEVT_CHOICE, &WaveformSettings::OnSelectionChanged, this);
	DisplayValues();
	AddPage(panel, _("General"));
}

void WaveformSettings::DoReset()
{
	Settings::reset_waveform();
}

bool WaveformSettings::DoOk()
{
	String category("waveform");
	String scaling;

	switch (scaling_choice->GetSelection())
	{
		case 0:
		{
			Settings::set_value(category, "scaling", "global");
			Settings::set_value(category, "magnitude", 1.0);
			break;
		}
		case 1:
		{
			Settings::set_value(category, "scaling", "local");
			Settings::set_value(category, "magnitude", 1.0);
			break;
		}
		default:
		{
			bool ok;
			String value = scaling_ctrl->GetValue();
			auto mag = value.to_float(&ok);
			if (!ok || mag <= 0 || mag > 1.0)
			{
				wxMessageBox(_("Invalid magnitude"), _("Invalid settings"), wxICON_ERROR);
				return false;
			}
			Settings::set_value(category, "scaling", "fixed");
			Settings::set_value(category, "magnitude", mag);
		}
	}

	return true;
}

void WaveformSettings::DisplayValues()
{
	String category("waveform");
	auto scaling = Settings::get_string(category, "scaling");

	if (scaling == "global")
	{
		scaling_choice->SetSelection(0);
		scaling_ctrl->SetValue(wxString());
	}
	else if (scaling == "local")
	{
		scaling_choice->SetSelection(1);
		scaling_ctrl->SetValue(wxString());
	}
	else if (scaling == "fixed")
	{
		scaling_choice->SetSelection(2);
		auto mag = Settings::get_number(category, "magnitude");
		scaling_ctrl->SetValue(wxString::Format("%.4f", mag));
	}
	else
	{
		throw error("Unrecognized waveform scaling option: %", scaling);
	}

	scaling_ctrl->Enable(scaling_choice->GetSelection() == 2);
}

void WaveformSettings::OnSelectionChanged(wxCommandEvent &e)
{
	scaling_ctrl->Enable(e.GetSelection() == 2);

	if (e.GetSelection() == 2)
	{
		auto mag = Settings::get_number("waveform", "magnitude");
		scaling_ctrl->SetValue(wxString::Format("%.4f", mag));
	}
	else
	{
		scaling_ctrl->SetValue(wxString());
	}
}
} // namespace phonometrica
