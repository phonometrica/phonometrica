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
 * Created: 20/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Base class for all sound plots (waveform, spectrogram, pitch and intensity tracks).                        *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SOUND_PLOT_HPP
#define PHONOMETRICA_SOUND_PLOT_HPP

#include <wx/dcclient.h>
#include <wx/graphics.h>
#include <phon/gui/plot/time_window.hpp>
#include <phon/application/sound.hpp>

namespace phonometrica {

class SoundPlot : public TimeAlignedWindow
{
public:

	SoundPlot(wxWindow *parent, const Handle<Sound> &snd);

	void SetSelection(PixelSelection sel);

	void ZoomToSelection();

	void ZoomIn();

	void ZoomOut();

	void ViewAll();

	void MoveForward();

	void MoveBackward();

	void EnableMouseTracking(bool value);

	Signal<PixelSelection> update_selection;

protected:

	bool HasSelection() const { return m_sel.from >= 0; }

	void OnStartSelection(wxMouseEvent &e);

	void OnEndSelection(wxMouseEvent &e);

	void OnMotion(wxMouseEvent &e);

	void OnLeaveWindow(wxMouseEvent &e);

	void OnMouseWheel(wxMouseEvent &e);

	TimeSpan ComputeZoomIn() const;

	TimeSpan ComputeZoomOut() const;

    double ClipTime(double t) const;

	Handle<Sound> m_sound;

	// Current selection on screen
	PixelSelection m_sel;

	// Start of the selection when the user clicks on the wavebar.
	double m_sel_start = -1;

	bool m_track_mouse;

};

} // namespace phonometrica



#endif // PHONOMETRICA_SOUND_PLOT_HPP
