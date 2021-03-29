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
 * Created: 27/03/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <phon/gui/pref/spectrogram_settings.hpp>
#include <phon/gui/sizer.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

SpectrogramSettings::SpectrogramSettings(wxWindow *parent) :
	PreferencesDialog(parent, _("Spectrogram settings..."))
{
	SetSize(wxSize(400, 550));
	auto panel = MakeGeneralPanel();
	AddPage(panel, _("General"));
}

void SpectrogramSettings::DoReset()
{
	Settings::reset_spectrogram();
}

bool SpectrogramSettings::DoOk()
{
	bool ok;
	String category("spectrogram");

	double window_size = 0.005;
	if (narrow_btn->GetValue())
		window_size = 0.025;
	else if (custom_btn->GetValue())
	{
		String sval = winlen_ctrl->GetValue();
		int value = sval.to_int(&ok);
		if (!ok || value <= 0)
		{
			wxMessageBox(_("Invalid window size"), _("Invalid setting"), wxICON_ERROR);
			return false;
		}
		window_size = double(value) / 1000;
	}

	intptr_t bandwidth;
	{
		bool ok;
		String value = bandwidth_ctrl->GetValue();
		bandwidth = value.to_int(&ok);
		if (!ok || bandwidth <= 0) {
			wxMessageBox(_("Invalid frequency range"), _("Invalid setting"), wxICON_ERROR);
			return false;
		}
	}

	String window_type = window_choice->GetStringSelection();
	if (window_type.empty()) {
		wxMessageBox(_("Invalid window type"), _("Invalid setting"), wxICON_ERROR);
			return false;
	}

	intptr_t threshold;
	{
		bool ok;
		String value = preemph_ctrl->GetValue();
		threshold = value.to_int(&ok);
		if (!ok || threshold < 0) {
			wxMessageBox(_("Invalid pre-emphasis threshold"), _("Invalid setting"), wxICON_ERROR);
			return false;
		}

	}

	Settings::set_value(category, "window_size", window_size);
	Settings::set_value(category, "frequency_range", bandwidth);
	Settings::set_value(category, "window_type", window_type);
	Settings::set_value(category, "dynamic_range", (intptr_t) dyn_range_slider->GetValue());
	Settings::set_value(category, "preemphasis_threshold", threshold);


	return true;
}

wxPanel *SpectrogramSettings::MakeGeneralPanel()
{
	auto panel = new wxPanel(m_book);
	auto sizer = new VBoxSizer;
	auto type_box = new wxStaticBox(panel, wxID_ANY, _("Spectrogram type"));
	auto box_sizer = new VBoxSizer;

	wide_btn = new wxRadioButton(type_box, wxID_ANY, _("Wide-band (5 ms)"));
	narrow_btn = new wxRadioButton(type_box, wxID_ANY, _("Narrow-band (25 ms)"));
	custom_btn = new wxRadioButton(type_box, wxID_ANY, _("Custom window size (in ms):"));
	winlen_ctrl = new wxTextCtrl(type_box, wxID_ANY);
	box_sizer->Add(wide_btn, 0, wxALL, 10);
	box_sizer->Add(narrow_btn, 0, wxLEFT|wxBOTTOM, 10);
	box_sizer->Add(custom_btn, 0, wxLEFT|wxBOTTOM, 10);
	box_sizer->Add(winlen_ctrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
	box_sizer->AddSpacer(20);
	type_box->SetSizer(box_sizer);


	sizer->Add(type_box, 0, wxEXPAND|wxALL, 10);
	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Frequency range (Hz):")), 0, wxLEFT|wxRIGHT|wxBOTTOM, 10);
	bandwidth_ctrl = new wxTextCtrl(panel, wxID_ANY);
	sizer->Add(bandwidth_ctrl, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);
	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Window type:")), 0, wxALL, 10);
	wxArrayString types;
	types.Add("Bartlett");
	types.Add("Blackman");
	types.Add("Gaussian");
	types.Add("Hamming");
	types.Add("Hann");
	types.Add("Rectangular");
	window_choice = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, types);
	sizer->Add(window_choice, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

	dyn_range_label = new wxStaticText(panel, wxID_ANY, _("Dynamic range"));
	sizer->Add(dyn_range_label, 0, wxLEFT|wxRIGHT|wxTOP, 10);
	dyn_range_slider = new wxSlider(panel, wxID_ANY, 1, 1, 255);
	sizer->Add(dyn_range_slider, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 10);

	sizer->Add(new wxStaticText(panel, wxID_ANY, _("Pre-emphasis threshold (Hz):")), 0, wxLEFT|wxRIGHT|wxTOP, 10);
	preemph_ctrl = new wxTextCtrl(panel, wxID_ANY);
	sizer->Add(preemph_ctrl, 0, wxEXPAND|wxALL, 10);

	panel->SetSizer(sizer);

	wide_btn->Bind(wxEVT_RADIOBUTTON, &SpectrogramSettings::OnDisableCustomBand, this);
	narrow_btn->Bind(wxEVT_RADIOBUTTON, &SpectrogramSettings::OnDisableCustomBand, this);
	custom_btn->Bind(wxEVT_RADIOBUTTON, &SpectrogramSettings::OnEnableCustomBand, this);
	dyn_range_slider->Bind(wxEVT_SLIDER, &SpectrogramSettings::OnDynamicRangeChanged, this);

	DisplayValues();

	return panel;
}

void SpectrogramSettings::OnEnableCustomBand(wxCommandEvent &)
{
	EnableCustomBand(true);
}

void SpectrogramSettings::OnDisableCustomBand(wxCommandEvent &)
{
	EnableCustomBand(false);
}

void SpectrogramSettings::EnableCustomBand(bool value)
{
	winlen_ctrl->Enable(value);
	if (!value) winlen_ctrl->SetValue(wxString());
}

void SpectrogramSettings::OnDynamicRangeChanged(wxCommandEvent &)
{
	SetDynamicRangeLabel(dyn_range_slider->GetValue());
}

void SpectrogramSettings::DisplayValues()
{
String category("spectrogram");

	auto range = (int) Settings::get_int(category, "dynamic_range");
	dyn_range_slider->SetValue(range);
	SetDynamicRangeLabel(range);

	auto bw = (int) Settings::get_int(category, "frequency_range");
	bandwidth_ctrl->SetValue(wxString::Format("%d", bw));

	double window_size = Settings::get_number(category, "window_size");

	if (window_size == 0.005)
	{
		EnableCustomBand(false);
		wide_btn->SetValue(true);
	}
	else if (window_size == 0.025)
	{
		EnableCustomBand(false);
		narrow_btn->SetValue(true);
	}
	else
	{
		custom_btn->SetValue(true);
		EnableCustomBand(true);
		winlen_ctrl->SetValue(wxString::Format("%.4f", window_size*1000));
	}

	auto window_type = Settings::get_string(category, "window_type");

	if (window_type == "Bartlett")
		window_choice->SetSelection(0);
	else if (window_type == "Blackman")
		window_choice->SetSelection(1);
	else if (window_type == "Gaussian")
		window_choice->SetSelection(2);
	else if (window_type == "Hamming")
		window_choice->SetSelection(3);
	else if (window_type == "Hann")
		window_choice->SetSelection(4);
	else if (window_type == "Rectangular")
		window_choice->SetSelection(5);
	else
		throw error("Invalid window type \"%\" in spectrogram settings");

	int threshold = (int) Settings::get_int(category, "preemphasis_threshold");
	preemph_ctrl->SetValue(wxString::Format("%d", threshold));
}

void SpectrogramSettings::SetDynamicRangeLabel(int value)
{
	auto label = wxString::Format(_("Dynamic range: %d dB"), value);
	dyn_range_label->SetLabel(label);
}
} // namespace phonometrica
