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
 * Purpose: View to display a sound file. Since annotation views derive from this class.                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SOUND_VIEW_HPP
#define PHONOMETRICA_SOUND_VIEW_HPP

#include <phon/gui/views/view.hpp>
#include <phon/gui/wave_bar.hpp>
#include <phon/gui/sound_zoom.hpp>
#include <phon/gui/plot/waveform.hpp>
#include <phon/gui/tool_bar.hpp>
#include <phon/gui/sizer.hpp>
#include <phon/application/sound.hpp>

namespace phonometrica {

class SoundView : public View
{
public:

	SoundView(wxWindow *parent, const Handle<Sound> &snd);

	void Initialize();

	bool IsModified() const override;

	void DiscardChanges() override;

	wxString GetLabel() const override;

	String GetPath() const override;

	void SetTimeSelection(double from, double to);

protected:

	void SetToolBar();

	void OnPlay(wxCommandEvent &);

	void OnStop(wxCommandEvent &);

	void OnHelp(wxCommandEvent &);

	virtual void ShowHelp();

	Handle<Sound> m_sound;

	ToolBar *m_toolbar;

	Array<SoundPlot*> m_plots;

	// Sizer for sound plots
	wxBoxSizer *m_inner_sizer;

	SoundZoom *m_zoom;

	WaveBar *m_wavebar;

};

} // namespace phonometrica



#endif // PHONOMETRICA_SOUND_VIEW_HPP
