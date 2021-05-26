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
 * Created: 19/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/msgdlg.h>
#include <phon/gui/views/sound_view.hpp>
#include <phon/gui/selection_dialog.hpp>
#include <phon/gui/pref/waveform_settings.hpp>
#include <phon/gui/pref/spectrogram_settings.hpp>
#include <phon/gui/pref/intensity_settings.hpp>
#include <phon/gui/pref/formant_settings.hpp>
#include <phon/gui/channel_dialog.hpp>
#include <phon/application/macros.hpp>
#include <phon/application/settings.hpp>
#include <phon/include/icons.hpp>

namespace phonometrica {

SoundView::SoundView(wxWindow *parent, const Handle<Sound> &snd) :
	View(parent), m_sound(snd), player(snd)
{
	snd->open();
	player.done.connect(&SoundView::OnPlayingDone, this);
	player.current_time.connect(&SoundView::SetTick, this);
}

void SoundView::Initialize()
{
	// Proportion for waveforms and spectrograms.
	const int large_prop = 3;
	SetToolBar();
	m_zoom = new SoundZoom(this);
	m_wavebar = new WaveBar(this, m_sound);
	m_x_axis = new XAxisInfo(this);
	m_y_axis = new YAxisInfo(this);

	// Packs the plots and wavebar
	m_inner_sizer = new VBoxSizer;
	m_inner_sizer->Add(m_x_axis, 0, wxEXPAND|wxRIGHT, 10);

	// Channel "0" represents the average of all the channels
	for (int i = 0; i <= m_sound->nchannel(); i++)
	{
		auto waveform = new Waveform(this, m_sound, i);
		waveforms.push_back(waveform);
		waveform->SetGlobalMagnitude(m_wavebar->GetMagnitude());
		m_inner_sizer->Add(waveform, large_prop, wxEXPAND|wxRIGHT, 10);
		auto hline = new HLine(this);
		m_inner_sizer->Add(hline);
		wave_lines.push_back(hline);
		m_plots.append(waveform);
	}
	for (int i = 0; i <= m_sound->nchannel(); i++)
	{
		auto spectrogram = new Spectrogram(this, m_sound, i);
		m_inner_sizer->Add(spectrogram, large_prop, wxEXPAND|wxRIGHT, 10);
		auto hline = new HLine(this);
		m_inner_sizer->Add(hline);
		spectrograms.push_back(spectrogram);
		spectrogram_lines.push_back(hline);
		m_plots.append(spectrogram);
	}
	for (int i = 0; i <= m_sound->nchannel(); i++)
	{
		auto track = new IntensityTrack(this, m_sound, i);
		m_inner_sizer->Add(track, 1, wxEXPAND|wxRIGHT, 10);
		auto hline = new HLine(this);
		m_inner_sizer->Add(hline);
		intensity_tracks.push_back(track);
		intensity_lines.push_back(hline);
		m_plots.append(track);
	}

	// Make all channels visible
	for (int i = 1; i <= m_sound->nchannel(); i++) {
		visible_channels.push_back(i);
	}

	ShowAverage(false);
	UpdatePlotLayout();
	m_inner_sizer->Add(m_zoom, 0, wxEXPAND|wxRIGHT, 10);
	m_inner_sizer->Add(m_wavebar, 0, wxEXPAND|wxRIGHT|wxBOTTOM, 10);
	m_msg_ctrl = new MessageCtrl(this);
	m_inner_sizer->Add(m_msg_ctrl, 0, wxEXPAND | wxRIGHT | wxLEFT | wxBOTTOM, 10);

	// Packs the y axis and the plots
	auto mid_sizer = new HBoxSizer;
	mid_sizer->Add(m_y_axis, 0, wxEXPAND, 0);
	mid_sizer->Add(m_inner_sizer, 1, wxEXPAND, 0);

	// Packs the toolbar and the mid sizer
	auto outer_sizer = new VBoxSizer;
	outer_sizer->Add(m_toolbar, 0, wxEXPAND|wxALL, 10);
	outer_sizer->Add(mid_sizer, 1, wxEXPAND|wxALL, 10);

	SetSizer(outer_sizer);

	m_wavebar->selection_changed.connect(&SoundZoom::OnSetSelection, m_zoom);
	for (auto plot : m_plots)
	{
		m_y_axis->AddWindow(plot);
		m_wavebar->change_window.connect(&SoundPlot::SetTimeWindow, plot);
		plot->update_window.connect(&SoundView::OnUpdateTimeWindow, this);
		plot->update_selection.connect(&SoundView::OnUpdateSelection, this);
		plot->invalidate_selection.connect(&SoundView::OnInvalidateSelection, this);
		plot->update_cursor.connect(&SoundView::OnUpdateCursor, this);
		plot->zoom_to_selection.connect(&SoundView::ZoomToSelection, this);
		plot->y_axis_modified.connect(&YAxisInfo::OnUpdate, m_y_axis);
		plot->update_status.connect(&MessageCtrl::SetStatus, m_msg_ctrl);
		plot->update_selection_status.connect(&MessageCtrl::SetSelection, m_msg_ctrl);
		plot->request_context_menu.connect(&SoundView::OnContextMenu, this);

		for (auto plot2 : m_plots)
		{
			plot->change_selection_state.connect(&SoundPlot::SetSelectionState, plot2);
			plot->change_selection_start.connect(&SoundPlot::SetSelectionStart, plot2);
		}
	}
	m_x_axis->invalidate_selection.connect(&SoundView::OnInvalidateSelection, this);
	m_y_axis->invalidate_selection.connect(&SoundView::OnInvalidateSelection, this);
	m_wavebar->change_window.connect(&XAxisInfo::SetTimeWindow, m_x_axis);
	m_wavebar->update_status.connect(&MessageCtrl::SetStatus, m_msg_ctrl);
	SetTopPlot();

	String category("sound_plots");
	bool show_wave = Settings::get_boolean(category, "waveform");
	bool show_spectrogram = Settings::get_boolean(category, "spectrogram");
	bool show_formants = Settings::get_boolean(category, "formants");
//	bool show_pitch = Settings::get_boolean(category, "pitch");
	bool show_intensity = Settings::get_boolean(category, "intensity");
	ShowWaveforms(show_wave);
	ShowSpectrogram(show_spectrogram);
	ShowFormants(show_formants);
	ShowIntensity(show_intensity);
	Layout();
}

bool SoundView::IsModified() const
{
	return m_sound->modified();
}

void SoundView::DiscardChanges()
{

}

wxString SoundView::GetLabel() const
{
	return m_sound->label();
}

void SoundView::SetToolBar()
{
#define ICN(x) wxBITMAP_PNG_FROM_DATA(x)
	m_toolbar = new ToolBar(this);
	auto save_tool = m_toolbar->AddButton(ICN(save), _("Save concordance... (" CTRL_KEY "S)"));
	save_tool->Disable();
	m_toolbar->AddSeparator();
	m_play_icon = ICN(play);
	m_play_sel_icon = ICN(play_selection);
	m_pause_icon = ICN(pause);
	m_pause_sel_icon = ICN(pause_selection);
	m_play_tool = m_toolbar->AddButton(m_play_icon, _("Play current window"));
	m_play_sel_tool = m_toolbar->AddButton(m_play_sel_icon, _("Play selection"));
	m_play_sel_tool->Enable(false);
	auto stop_tool = m_toolbar->AddButton(ICN(stop), _("Stop playing"));
	m_toolbar->AddSeparator();

	auto backward_tool = m_toolbar->AddButton(ICN(back), _("Shift window backward"));
	auto forward_tool = m_toolbar->AddButton(ICN(next), _("Shift window forward"));
	auto zoom_out_tool = m_toolbar->AddButton(ICN(zoom_minus), _("Zoom out"));
	auto zoom_in_tool = m_toolbar->AddButton(ICN(zoom_plus), _("Zoom in"));
	auto zoom_sel_tool = m_toolbar->AddButton(ICN(collapse), _("Zoom to selection"));
	auto zoom_all_tool = m_toolbar->AddButton(ICN(expand), _("View whole file"));
	auto sel_tool = m_toolbar->AddButton(ICN(selection), _("Select window"));
	m_toolbar->AddSeparator();

	m_wave_tool = m_toolbar->AddMenuButton(ICN(waveform), _("Waveform settings"));
	m_spectrum_tool = m_toolbar->AddMenuButton(ICN(spectrum), _("Spectrogram settings"));
	m_formant_tool = m_toolbar->AddMenuButton(ICN(formants), _("Formants settings"));
	m_pitch_tool = m_toolbar->AddMenuButton(ICN(pitch), _("Pitch settings"));
	m_intensity_tool = m_toolbar->AddMenuButton(ICN(intensity), _("Intensity settings"));
	m_toolbar->AddSeparator();

	auto channel_tool = m_toolbar->AddButton(ICN(layout), _("Select visible channels..."));

	m_mouse_tool = m_toolbar->AddToggleButton(ICN(mouse), _("Enable mouse tracking"));
	m_mouse_tool->Check(Settings::get_boolean("enable_mouse_tracking"));

	m_toolbar->AddStretchableSpace();
	auto help_tool = m_toolbar->AddHelpButton();
#undef ICN

	m_play_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnPlayWindow, this);
	m_play_sel_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnPlaySelection, this);
	stop_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnStop, this);
	help_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnHelp, this);
	forward_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnMoveForward, this);
	backward_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnMoveBackward, this);
	zoom_in_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnZoomIn, this);
	zoom_out_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnZoomOut, this);
	zoom_sel_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnZoomToSelection, this);
	zoom_all_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnViewAll, this);
	sel_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnSelectWindow, this);
	channel_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnSelectChannels, this);
	m_wave_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnWaveMenu, this);
	m_spectrum_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnSpectrogramMenu, this);
	m_formant_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnFormantsMenu, this);
	m_pitch_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnPitchMenu, this);
	m_intensity_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnIntensityMenu, this);
	m_mouse_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnEnableMouseTracking, this);
}

String SoundView::GetPath() const
{
	return m_sound->path();
}

void SoundView::SetTimeSelection(double from, double to)
{
	if (to > from + m_sound->duration()) {
		to = from + m_sound->duration();
	}
	for (auto plot : m_plots) {
		plot->SetTimeWindow({from, to});
	}
	UpdateTimeWindow(TimeWindow{from, to});
}

void SoundView::OnPlayWindow(wxCommandEvent &)
{
    if (player.running())
    {
        if (player.paused())
        {
			SetPauseWindowIcon();
            player.resume();
        }
        else
        {
			SetPlayWindowIcon();
            player.pause();
        }
    }
    else
    {
        auto times = GetFirstPlot()->GetTimeWindow();
		SetPauseWindowIcon();
		try {
			player.play(times.first, times.second);
			if (player.has_error())
			{
				player.raise_error();
			}
		}
		catch (std::exception &e)
		{
			auto msg = wxString::Format(_("Cannot play sound window: %s"), e.what());
			wxMessageBox(msg, _("Sound error"), wxICON_ERROR);
		}
    }
}

void SoundView::OnPlaySelection(wxCommandEvent &)
{
	if (player.running())
	{
		if (player.paused())
		{
			SetPauseSelectionIcon();
			player.resume();
		}
		else
		{
			SetPlaySelectionIcon();
			player.pause();
		}
	}
	else
	{
		auto times = GetFirstPlot()->GetSelection();
		SetPauseSelectionIcon();
		try {
			player.play(times.t1, times.t2);
			if (player.has_error())
			{
				player.raise_error();
			}
		}
		catch (std::exception &e)
		{
			auto msg = wxString::Format(_("Cannot play sound selection: %s"), e.what());
			wxMessageBox(msg, _("Sound error"), wxICON_ERROR);
		}
	}
}

void SoundView::OnStop(wxCommandEvent &)
{
    player.interrupt();
	SetPlayWindowIcon();
	SetPlaySelectionIcon();
    HideTick();
}

void SoundView::OnHelp(wxCommandEvent &)
{
	ShowHelp();
}

void SoundView::ShowHelp()
{
	auto url = Settings::get_documentation_page("sound.html");
	wxLaunchDefaultBrowser(url, wxBROWSER_NOBUSYCURSOR);
}

void SoundView::OnMoveForward(wxCommandEvent &)
{
	for (auto plot : m_plots) {
		plot->MoveForward();
	}
	auto win = GetTimeWindow();
	UpdateTimeWindow(win);
}

void SoundView::OnMoveBackward(wxCommandEvent &)
{
	for (auto plot : m_plots) {
		plot->MoveBackward();
	}
	auto win = GetTimeWindow();
	UpdateTimeWindow(win);
}

void SoundView::OnZoomIn(wxCommandEvent &)
{
	for (auto plot : m_plots) {
		plot->ZoomIn();
	}
	auto win = GetTimeWindow();
	UpdateTimeWindow(win);
}

void SoundView::OnZoomOut(wxCommandEvent &)
{
	for (auto plot : m_plots) {
		plot->ZoomOut();
	}
	auto win = GetTimeWindow();
	UpdateTimeWindow(win);
}

void SoundView::OnZoomToSelection(wxCommandEvent &)
{
	ZoomToSelection();
}

void SoundView::ZoomToSelection()
{
	// This function is also called by plots when clicking on the middle button.
	for (auto plot : m_plots) {
		plot->ZoomToSelection();
	}
	auto win = GetTimeWindow();
	UpdateTimeWindow(win);
}

void SoundView::OnViewAll(wxCommandEvent &)
{
	for (auto plot : m_plots) {
		plot->ViewAll();
	}
	auto win = GetTimeWindow();
	UpdateTimeWindow(win);
}

void SoundView::OnSelectWindow(wxCommandEvent &)
{
	try
	{
		SelectionDialog dlg(this);
		dlg.Move(wxGetMousePosition());

		if (dlg.ShowModal() == wxID_OK)
		{
			auto win = dlg.GetSelection();

			for (auto plot : m_plots) {
				plot->SetTimeWindow(win);
			}
			UpdateTimeWindow(win);
		}
	}
	catch (std::exception &e)
	{
		wxMessageBox(e.what(), _("Selection error"), wxICON_ERROR);
	}
}

void SoundView::OnUpdateTimeWindow(TimeWindow win)
{
	for (auto plot : m_plots) {
		plot->SetTimeWindow(win);
	}
	UpdateTimeWindow(win);
}

void SoundView::OnUpdateSelection(const TimeSelection &sel)
{
	for (auto plot : m_plots) {
		plot->SetSelection(sel);
	}
	UpdateXAxisSelection(sel);

	if (!m_play_sel_tool->IsEnabled()) {
		m_play_sel_tool->Enable();
	}
}

void SoundView::OnInvalidateSelection()
{
	for (auto plot : m_plots) {
		plot->InvalidateSelection();
	}
	m_x_axis->InvalidateSelection();
	m_msg_ctrl->ClearSelection();
	m_play_sel_tool->Enable(false);
}

void SoundView::UpdateXAxisSelection(const TimeSelection &sel)
{
	m_x_axis->SetSelection(sel);
}

void SoundView::UpdateTimeWindow(TimeWindow win)
{
	m_wavebar->SetTimeSelection(win);
	m_x_axis->SetTimeWindow(win);
	UpdateXAxisSelection(GetFirstPlot()->GetSelection());
}

SoundPlot *SoundView::GetFirstPlot() const
{
	return m_plots.first();
}

TimeWindow SoundView::GetTimeWindow() const
{
	return GetFirstPlot()->GetTimeWindow();
}

void SoundView::OnWaveMenu(wxCommandEvent &)
{
	auto menu = new wxMenu;
	auto show_msg = m_sound->is_mono() ? _("Show waveform") : _("Show waveforms");
	auto show_tool = menu->AppendCheckItem(wxID_ANY, show_msg);
	menu->AppendSeparator();
	auto settings_tool = menu->Append(wxID_ANY, _("Waveform settings..."));
	show_tool->Check(true);
	show_tool->Enable(false);
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnShowWaveforms, this, show_tool->GetId());
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnWaveformSettings, this, settings_tool->GetId());

	m_toolbar->ShowMenu(m_wave_tool, menu);
}

void SoundView::OnSpectrogramMenu(wxCommandEvent &)
{
	auto menu = new wxMenu;
	auto show_tool = menu->AppendCheckItem(wxID_ANY, _("Show spectrogram"));
	menu->AppendSeparator();
	auto settings_tool = menu->Append(wxID_ANY, _("Spectrogram settings..."));
	bool show = Settings::get_boolean("sound_plots", "spectrogram");
	show_tool->Check(show);
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnShowSpectrogram, this, show_tool->GetId());
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnSpectrogramSettings, this, settings_tool->GetId());

	m_toolbar->ShowMenu(m_spectrum_tool, menu);
}

void SoundView::OnFormantsMenu(wxCommandEvent &)
{
	String category("sound_plots");
	auto menu = new wxMenu;
	auto show_tool = menu->AppendCheckItem(wxID_ANY, _("Show formants"));
	auto get_tool = menu->Append(wxID_ANY, _("Get formants"));
	menu->AppendSeparator();
	auto settings_tool = menu->Append(wxID_ANY, _("Formant settings..."));
	bool show = Settings::get_boolean(category, "formants");
	show_tool->Check(show);
	show_tool->Enable(Settings::get_boolean(category, "spectrogram"));
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnShowFormants, this, show_tool->GetId());
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnGetFormants, this, get_tool->GetId());
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnFormantSettings, this, settings_tool->GetId());

	m_toolbar->ShowMenu(m_formant_tool, menu);
}

void SoundView::OnPitchMenu(wxCommandEvent &)
{

}

void SoundView::OnIntensityMenu(wxCommandEvent &)
{
	String category("sound_plots");
	auto menu = new wxMenu;
	auto show_tool = menu->AppendCheckItem(wxID_ANY, _("Show intensity"));
	auto get_tool = menu->Append(wxID_ANY, _("Get intensity"));
	menu->AppendSeparator();
	auto settings_tool = menu->Append(wxID_ANY, _("Intensity settings..."));
	bool show = Settings::get_boolean(category, "intensity");
	show_tool->Check(show);
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnShowIntensity, this, show_tool->GetId());
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnGetIntensity, this, get_tool->GetId());
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnIntensitySettings, this, settings_tool->GetId());

	m_toolbar->ShowMenu(m_intensity_tool, menu);
}

void SoundView::OnEnableMouseTracking(wxCommandEvent &)
{
	m_mouse_tool->Toggle();
	auto value = m_mouse_tool->IsChecked();
	Settings::set_value("enable_mouse_tracking", value);

	for (auto plot : m_plots) {
		plot->EnableMouseTracking(value);
	}
}

void SoundView::OnUpdateCursor(double pos)
{
	for (auto plot : m_plots) {
		plot->SetCursorPosition(pos);
	}
}

void SoundView::SetTopPlot()
{
	// Unmark previous top
	for (auto plot : m_plots)
	{
		if (plot->IsTop())
		{
			plot->MakeTop(false);
			break;
		}
	}
	// Set new top plot
	for (auto plot : m_plots)
	{
		if (plot->IsShown())
		{
			plot->MakeTop(true);
			break;
		}
	}
}

void SoundView::SetPauseWindowIcon()
{
	m_play_tool->SetBitmap(m_pause_icon);
}

void SoundView::SetPlayWindowIcon()
{
	m_play_tool->SetBitmap(m_play_icon);
}

void SoundView::SetPlaySelectionIcon()
{
	m_play_sel_tool->SetBitmap(m_play_sel_icon);
}

void SoundView::SetPauseSelectionIcon()
{
	m_play_sel_tool->SetBitmap(m_pause_sel_icon);
}

void SoundView::HideTick()
{
	SetTick(-1.0);
}

void SoundView::OnPlayingDone()
{
	SetPlayWindowIcon();
	SetPlaySelectionIcon();
	HideTick();
}

void SoundView::SetTick(double t)
{
	for (auto plot : m_plots)
	{
		plot->SetTick(t);
	}
}

void SoundView::OnWaveformSettings(wxCommandEvent &)
{
	WaveformSettings ed(this);

	if (ed.ShowModal() != wxID_CANCEL)
	{
		for (auto wav : waveforms) {
			wav->UpdateSettings();
		}
	}
}

void SoundView::OnSpectrogramSettings(wxCommandEvent &)
{
	SpectrogramSettings ed(this);

	if (ed.ShowModal() != wxID_CANCEL)
	{
		for (auto spectrogram : spectrograms) {
			spectrogram->UpdateSettings();
		}
	}
}

void SoundView::OnFormantSettings(wxCommandEvent &)
{
	FormantSettings ed(this);

	if (ed.ShowModal() != wxID_CANCEL)
	{
		for (auto spectrogram : spectrograms) {
			spectrogram->UpdateSettings();
		}
	}
}

void SoundView::OnPitchSettings(wxCommandEvent &)
{

}

void SoundView::OnIntensitySettings(wxCommandEvent &)
{
	IntensitySettings ed(this);

	if (ed.ShowModal() != wxID_CANCEL)
	{
		for (auto track : intensity_tracks) {
			track->UpdateSettings();
		}
	}
}

void SoundView::OnShowSpectrogram(wxCommandEvent &e)
{
	ShowSpectrogram(e.IsChecked());
}

void SoundView::ShowSpectrogram(bool value)
{
	Settings::set_value("sound_plots", "spectrogram", value);

	for (size_t i = 0; i < spectrograms.size(); i++) {
		spectrograms[i]->SetPlotVisible(value);
	}
	UpdatePlotLayout();
}

void SoundView::OnShowFormants(wxCommandEvent &e)
{
	ShowFormants(e.IsChecked());
	UpdatePlotLayout();
}

void SoundView::OnShowIntensity(wxCommandEvent &e)
{
	ShowIntensity(e.IsChecked());
	UpdatePlotLayout();
}

void SoundView::ShowFormants(bool value)
{
	if (value) {
		ShowSpectrogram(true);
	}
	for (auto spectrogram: spectrograms) {
		spectrogram->ShowFormants(value);
	}
	Settings::set_value("sound_plots", "formants", value);
}

void SoundView::ShowIntensity(bool value)
{
	for (auto track : intensity_tracks) {
		track->SetPlotVisible(value);
	}
	Settings::set_value("sound_plots", "intensity", value);
}

void SoundView::OnShowWaveforms(wxCommandEvent &e)
{
	ShowWaveforms(e.IsChecked());
}

void SoundView::ShowWaveforms(bool value)
{
	Settings::set_value("sound_plots", "spectrogram", value);
	for (auto wave : waveforms) {
		wave->SetPlotVisible(value);
	}
	UpdatePlotLayout();
}

void SoundView::OnGetFormants(wxCommandEvent &)
{
	if (!spectrograms[0]->HasSelection()) {
		wxMessageBox(_("First select a point or a portion of the signal"), _("Cannot measure formants"), wxICON_ERROR);
		return;
	}
	auto sel = spectrograms[0]->GetSelection();
	String cmd = (sel.t1 == sel.t2) ? String::format("report_formants(%.10f)", sel.t1) : String::format("report_formants(%.10f, %.10f)", sel.t1, sel.t2);
	SendCommand(cmd);
}

void SoundView::OnGetIntensity(wxCommandEvent &)
{
	if (!intensity_tracks[0]->HasSelection()) {
		wxMessageBox(_("First select a point"), _("Cannot measure intensity"), wxICON_ERROR);
		return;
	}

	auto sel = intensity_tracks[0]->GetSelection();

	if (sel.t1 != sel.t2) {
		wxMessageBox(_("First select a point"), _("Cannot measure intensity"), wxICON_ERROR);
		return;
	}
	String cmd = String::format("report_intensity(%.10f)", sel.t1);
	SendCommand(cmd);
}

void SoundView::SendCommand(const String &code)
{
	request_console();
	send_code(code);
}

Handle<Sound> SoundView::GetSound() const
{
	return m_sound;
}

void SoundView::OnSelectChannels(wxCommandEvent &)
{
	ChannelDialog ed(this, m_sound->nchannel(), visible_channels);

	if (ed.ShowModal() == wxID_OK)
	{
		visible_channels = ed.GetSelectedChannels();

		if (visible_channels.empty())
		{
			ShowAverage(true);
			for (int i = 1; i < (int)waveforms.size(); i++) {
				ShowChannel(i, false);
			}
		}
		else
		{
			ShowAverage(false);
			for (int i = 1; i < (int)waveforms.size(); i++)
			{
				bool show = (std::find(visible_channels.begin(), visible_channels.end(), i) != visible_channels.end());
				ShowChannel(i, show);
			}
		}
		UpdatePlotLayout();
	}
}

void SoundView::ShowAverage(bool show)
{
	ShowChannel(0, show);
}

void SoundView::ShowChannel(int channel, bool show)
{
	waveforms[channel]->SetChannelVisible(show);
	spectrograms[channel]->SetChannelVisible(show);
	intensity_tracks[channel]->SetChannelVisible(show);
}

void SoundView::UpdatePlotLayout()
{
	for (int i = 0; i <= m_sound->nchannel(); i++)
	{
		bool show = waveforms[i]->IsVisible();
		waveforms[i]->Show(show);
		wave_lines[i]->Show(show);
		show = spectrograms[i]->IsVisible();
		spectrograms[i]->Show(show);
		spectrogram_lines[i]->Show(show);
		show = intensity_tracks[i]->IsVisible();
		intensity_tracks[i]->Show(show);
		intensity_lines[i]->Show(show);
	}
	Layout();
}

void SoundView::OnContextMenu(wxPoint pos)
{
	auto menu = new wxMenu;
	auto play_entry = menu->Append(wxNewId(), _("Play current window"));
	auto play_sel_entry = menu->Append(wxNewId(), _("Play selection"));
	auto zoom_sel_entry = menu->Append(wxNewId(), _("Zoom to selection"));
	menu->AppendSeparator();
	auto clear_sel_entry = menu->Append(wxNewId(), _("Clear selection"));

	if (!GetFirstPlot()->HasSelection())
	{
		play_sel_entry->Enable(false);
		zoom_sel_entry->Enable(false);
		clear_sel_entry->Enable(false);
	}

	Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnPlayWindow, this, play_entry->GetId());
	Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnPlaySelection, this, play_sel_entry->GetId());
	Bind(wxEVT_COMMAND_MENU_SELECTED, &SoundView::OnZoomToSelection, this, zoom_sel_entry->GetId());
	Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent &) { OnInvalidateSelection(); }, clear_sel_entry->GetId());


	PopupMenu(menu, ScreenToClient(pos));
}

Array<int> SoundView::GetVisibleChannels() const
{
	Array<int> result;

	for (size_t i = 0; i < waveforms.size(); i++)
	{
		if (waveforms[i]->IsChannelVisible()) {
			result.append((int)i);
		}
	}

	return result;
}

} // namespace phonometrica
