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

	PixelSelection GetSelection() const;

	void SetSelection(PixelSelection sel);

	void SetCursorPosition(double pos);

	void SetAnchor(TimeAnchor anchor);

	void ZoomToSelection();

	void ZoomIn();

	void ZoomOut();

	void ViewAll();

	void MoveForward();

	void MoveBackward();

	void EnableMouseTracking(bool value);

	bool IsTop() const;

	void MakeTop(bool value);

	void SetTimeWindow(TimeSpan win) override;

	Signal<PixelSelection> update_selection;

	Signal<double> update_cursor;

	Signal<TimeAnchor> update_anchor;

	Signal<> zoom_to_selection;

protected:

	enum class SelectionState : char
	{
		Inactive,
		Started,
		Active
	};

	bool HasSelection() const { return m_sel.first >= 0; }

	bool HasTimeAnchor() const { return m_anchor.first >= 0; }

	bool HasCursor() const { return m_cursor_pos >= 0; }

	void DrawSelection(wxPaintDC &dc);

	void DrawTimeAnchor(wxPaintDC &dc);

	void DrawCursor(wxPaintDC &dc);

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
	PixelSelection m_sel = {-1.0, -1.0};

	// Start of the selection when the user clicks on the wavebar.
	double m_sel_start = -1;

	TimeAnchor m_anchor = {-1, -1.0};

	double m_cursor_pos = -1;

	bool m_track_mouse = false;

	bool m_is_top = false;

	SelectionState m_sel_state = SelectionState::Inactive;
};

} // namespace phonometrica



#endif // PHONOMETRICA_SOUND_PLOT_HPP
