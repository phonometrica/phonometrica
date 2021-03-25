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
 * Purpose: scrollbar that displays the whole sound file in a sound view or annotation view.                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_WAVE_BAR_HPP
#define PHONOMETRICA_WAVE_BAR_HPP

#include <wx/dcclient.h>
#include <wx/window.h>
#include <wx/graphics.h>
#include <phon/gui/helpers.hpp>
#include <phon/application/sound.hpp>
#include <phon/utils/signal.hpp>

namespace phonometrica {

class WaveBar final : public wxWindow
{
public:

	WaveBar(wxWindow *parent, const Handle <Sound> &snd);

	void SetTimeSelection(TimeWindow win);

	Signal<PixelSelection> selection_changed;

	Signal<TimeWindow> change_window;

private:

	void OnPaint(wxPaintEvent &);

	void Render(wxPaintDC &dc);

	void UpdateCache();

	double SampleToYPos(double s) const;

	bool HasSelection() const { return m_sel.first >= 0; }

	double TimeToXPos(double t) const;

	double XPosToTime(double x) const;

	void SetSelection(PixelSelection sel);

	void OnStartSelection(wxMouseEvent &e);

	void OnEndSelection(wxMouseEvent &e);

	void OnMotion(wxMouseEvent &e);

	void OnMouseWheel(wxMouseEvent &e);

	void MoveForward();

	void MoveBackward();

	Handle<Sound> m_sound;

	// To avoid recomputing the data on every paint event, we cache it here. We only
	// recompute if if the size of the window has changed.
	std::vector<std::pair<double,double>> m_cache;

	// Current selection
	PixelSelection m_sel;

	// Cached magnitude to normalize amplitudes (computed once since height is fixed).
	double raw_magnitude = 0.0;

	// Start of the selection when the user clicks on the wavebar.
	double m_sel_start = -1;
};

} // namespace phonometrica



#endif // PHONOMETRICA_WAVE_BAR_HPP
