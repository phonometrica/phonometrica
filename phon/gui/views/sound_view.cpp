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

#include <phon/gui/views/sound_view.hpp>
#include <phon/gui/plot_separator.hpp>
#include <phon/application/macros.hpp>
#include <phon/application/settings.hpp>
#include <phon/include/icons.hpp>

namespace phonometrica {

SoundView::SoundView(wxWindow *parent, const Handle<Sound> &snd) :
	View(parent), m_sound(snd)
{
	snd->open();
}

void SoundView::Initialize()
{
	SetToolBar();
	m_zoom = new SoundZoom(this);
	m_wavebar = new WaveBar(this, m_sound);

	// Packs the plots and wavebar
	m_inner_sizer = new VBoxSizer;
	for (int i = 1; i <= m_sound->nchannel(); i++)
	{
		auto waveform = new Waveform(this, m_sound, i);
		m_inner_sizer->Add(waveform, 1, wxEXPAND|wxLEFT|wxRIGHT, 10);
		m_inner_sizer->Add(new PlotSeparator(this));
		m_plots.append(waveform);
	}
	m_inner_sizer->Add(m_zoom, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);
	m_inner_sizer->Add(m_wavebar, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);

	// Packs the y axis and the plots
	auto mid_sizer = new HBoxSizer;
	mid_sizer->AddSpacer(60); // This will have info on the left
	mid_sizer->Add(m_inner_sizer, 1, wxEXPAND, 0);

	// Packs the toolbar and the mid sizer
	auto outer_sizer = new VBoxSizer;
	outer_sizer->Add(m_toolbar, 0, wxEXPAND | wxALL, 10);
	outer_sizer->Add(mid_sizer, 1, wxEXPAND | wxALL, 10);

	SetSizer(outer_sizer);

	m_wavebar->selection_changed.connect(&SoundZoom::OnSetSelection, m_zoom);
	for (auto plot : m_plots) {
		m_wavebar->change_window.connect(&SoundPlot::ChangeWindow, plot);
	}

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
	auto play_tool = m_toolbar->AddButton(ICN(play), _("Play window or selection"));
	auto stop_tool = m_toolbar->AddButton(ICN(stop), _("Stop playing"));
	m_toolbar->AddStretchableSpace();
	auto help_tool = m_toolbar->AddHelpButton();
#undef ICN

	play_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnPlay, this);
	stop_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnStop, this);
	help_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SoundView::OnHelp, this);
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
		plot->ChangeWindow({from, to});
	}
	m_wavebar->SetTimeSelection({from, to});
}

void SoundView::OnPlay(wxCommandEvent &)
{

}

void SoundView::OnStop(wxCommandEvent &)
{

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


} // namespace phonometrica
