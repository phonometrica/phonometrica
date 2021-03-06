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
 * Created: 27/05/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Base class for sound and annotation views.                                                                 *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SPEECH_VIEW_HPP
#define PHONOMETRICA_SPEECH_VIEW_HPP

#include <phon/gui/views/view.hpp>
#include <phon/gui/wave_bar.hpp>
#include <phon/gui/sound_zoom.hpp>
#include <phon/gui/plot/waveform.hpp>
#include <phon/gui/tool_bar.hpp>
#include <phon/gui/x_axis_info.hpp>
#include <phon/gui/y_axis_info.hpp>
#include <phon/gui/sizer.hpp>
#include <phon/gui/lines.hpp>
#include <phon/gui/message_ctrl.hpp>
#include <phon/gui/plot/spectrogram.hpp>
#include <phon/gui/plot/intensity_track.hpp>
#include <phon/application/audio_player.hpp>
#include <phon/application/sound.hpp>

namespace phonometrica {

class SpeechView : public View
{
public:

	SpeechView(wxWindow *parent, const Handle<Sound> &snd);

	void Initialize();

	bool IsModified() const override;

	void DiscardChanges() override;

	void SetTimeWindow(double from, double to);

	Handle<Sound> GetSound() const;

	Array<int> GetVisibleChannels() const;

	double GetWindowDuration() const;

	double GetSelectionDuration() const;

protected:

	virtual void AddAnnotationMenu(ToolBar *toolbar) { };

	virtual void AddAnnotationLayers(wxSizer *sizer) { };

	virtual void ShowHelp();

	void SetToolBar();

	void UpdateAuxiliaryTimes(TimeWindow win);

	virtual void UpdateLayersWindow(TimeWindow win) { }

	SoundPlot *GetFirstPlot() const;

	TimeWindow GetTimeWindow() const;

	void OnPlayWindow(wxCommandEvent &);

	void OnPlaySelection(wxCommandEvent &);

	void OnStop(wxCommandEvent &);

	void OnHelp(wxCommandEvent &);

	void OnMoveForward(wxCommandEvent &);

	void OnMoveBackward(wxCommandEvent &);

	void OnZoomIn(wxCommandEvent &);

	void OnZoomOut(wxCommandEvent &);

	void OnZoomToSelection(wxCommandEvent &);

	void ZoomToSelection();

	void OnViewAll(wxCommandEvent &);

	void OnSelectWindow(wxCommandEvent &);

	void OnSelectChannels(wxCommandEvent &);

	void OnWaveMenu(wxCommandEvent &);

	void OnSpectrogramMenu(wxCommandEvent &);

	void OnFormantsMenu(wxCommandEvent &);

	void OnPitchMenu(wxCommandEvent &);

	void OnIntensityMenu(wxCommandEvent &);

	void OnEnableMouseTracking(wxCommandEvent &);

	void OnUpdateTimeWindow(TimeWindow win);

	void OnUpdateSelection(const TimeSelection &sel);

	void OnInvalidateSelection();

	void UpdateXAxisSelection(const TimeSelection &sel);

	void OnUpdateCursor(double pos);

	void SetTopPlot();

	void SetPlayWindowIcon();

	void SetPauseWindowIcon();

	void SetPlaySelectionIcon();

	void SetPauseSelectionIcon();

	void HideTick();

	void OnPlayingDone();

	void SetTick(double t);

	void OnWaveformSettings(wxCommandEvent &);

	void OnSpectrogramSettings(wxCommandEvent &);

	void OnFormantSettings(wxCommandEvent &);

	void OnPitchSettings(wxCommandEvent &);

	void OnIntensitySettings(wxCommandEvent &);

	void OnShowSpectrogram(wxCommandEvent &e);

	void OnShowFormants(wxCommandEvent &e);

	void ShowSpectrogram(bool value);

	void ShowFormants(bool value);

	void OnShowIntensity(wxCommandEvent &e);

	void ShowIntensity(bool value);

	void OnShowWaveforms(wxCommandEvent &e);

	void ShowWaveforms(bool value);

	void OnGetFormants(wxCommandEvent &);

	void OnGetIntensity(wxCommandEvent &);

	void SendCommand(const String &code);

	void ShowAverage(bool show);

	void ShowChannel(int channel, bool show);

	void UpdatePlotLayout();

	void OnContextMenu(wxPoint pos);

	wxButton *m_wave_tool, *m_spectrum_tool, *m_formant_tool, *m_pitch_tool, *m_intensity_tool;

	wxButton *m_play_tool, *m_play_sel_tool, *m_zoom_sel_tool;

	wxBitmap m_play_icon, m_play_sel_icon, m_pause_icon, m_pause_sel_icon;

	ToggleButton *m_mouse_tool;

	Handle<Sound> m_sound;

	ToolBar *m_toolbar;

	// Keep track of all the sound plots, in the order they are created.
	Array<SoundPlot*> m_plots;

	// Keep track of all the speech widgets (sound plots + layers).
	Array<SpeechWidget *> m_speech_widgets;

	XAxisInfo *m_x_axis;

	YAxisInfo *m_y_axis;

	// Sizer for sound plots
	wxBoxSizer *m_inner_sizer;

	SoundZoom *m_zoom;

	WaveBar *m_wavebar;

	AudioPlayer player;

	std::vector<Waveform*> waveforms;

	std::vector<Spectrogram*> spectrograms;

	std::vector<IntensityTrack*> intensity_tracks;

	std::vector<HLine*> wave_lines, spectrogram_lines, pitch_lines, intensity_lines;

	std::vector<int> visible_channels;

	MessageCtrl *m_msg_ctrl;
};


} // namespace phonometrica

#endif // PHONOMETRICA_SPEECH_VIEW_HPP
