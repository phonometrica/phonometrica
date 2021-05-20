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
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <phon/gui/plot/time_window.hpp>
#include <phon/application/sound.hpp>

namespace phonometrica {

class SoundPlot : public TimeAlignedWindow
{
public:

	enum class SelectionState : char
	{
		Inactive,
		Started,
		Active
	};

	SoundPlot(wxWindow *parent, const Handle <Sound> &snd, int channel);

	bool HasSelection() const { return m_sel.t1 >= 0; }

	const TimeSelection & GetSelection() const;

	void SetSelection(const TimeSelection &sel);

	void InvalidateSelection();

	void SetCursorPosition(double pos);

	void ZoomToSelection();

	void ZoomIn();

	void ZoomOut();

	void ViewAll();

	void MoveForward();

	void MoveBackward();

	void EnableMouseTracking(bool value);

	bool IsTop() const;

	void MakeTop(bool value);

	void SetSelectionState(SelectionState value);

	void SetSelectionStart(double value);

	TimeWindow GetPlayWindow() const;

	void SetTick(double time);

	void UpdateSettings();

	bool IsChannelVisible() const;

	void SetChannelVisible(bool value);

	bool IsPlotVisible() const;

	void SetPlotVisible(bool value);

	bool IsVisible() const;

	Signal<const TimeSelection&> update_selection;

	Signal<> invalidate_selection;

	Signal<double> update_cursor;

	Signal<> zoom_to_selection;

	Signal<SelectionState> change_selection_state;

	Signal<double> change_selection_start;

	Signal<> y_axis_modified;

	Signal<const wxString&> update_status;

protected:

	virtual void UpdateCache() = 0;

	virtual wxString GetStatus() = 0;

	void OnPaint(wxPaintEvent &);

	void OnEraseBackground(wxEraseEvent &);

	void Render(wxBufferedPaintDC &dc);

	bool HasVisibleSelection() const;

	bool HasPointSelection() const { return m_sel.is_point(); }

	bool HasCursor() const { return m_cursor_pos >= 0; }

	void DrawSelection(wxBufferedPaintDC &dc);

	void DrawSpanSelection(wxBufferedPaintDC &dc);

	void DrawPointSelection(wxBufferedPaintDC &dc);

	void DrawCursor(wxBufferedPaintDC &dc);

	void OnStartSelection(wxMouseEvent &e);

	void OnEndSelection(wxMouseEvent &e);

	void DrawTimeTick(wxBufferedPaintDC &dc);

	void OnMotion(wxMouseEvent &e);

	void OnLeaveWindow(wxMouseEvent &e);

	void OnMouseWheel(wxMouseEvent &e);

	TimeWindow ComputeZoomIn() const;

	TimeWindow ComputeZoomOut() const;

    double ClipTime(double t) const;

    void InvalidateCache();

    bool HasValidCache() const;

    virtual void ReadSettings() = 0;

	Handle<Sound> m_sound;

	// Cached plot. The selection (if any) will be overlaid over this image.
	wxBitmap m_cached_bmp;

	// Cache the size of the plot when we compute the data
	wxSize m_cached_size;

	// Current selection on screen
	TimeSelection m_sel = {-1.0, -1.0 };

	// Start of the selection when the user clicks on the wavebar.
	double m_sel_start = -1.0;

	double m_cursor_pos = -1.0;

	// Time tick when a sound is being played.
	double m_tick_time = -1.0;

	bool m_track_mouse = false;

	bool m_is_top = false;

	// Is this channel selected by the user?
	bool m_channel_visible = true;

	// Is this type of plot (waveform, spectrogram, etc.) selected by the user?
	bool m_plot_visible = true;

	SelectionState m_sel_state = SelectionState::Inactive;

	// Channel associated with this plot (0 is the the average of all channels)
	int m_channel;
};

} // namespace phonometrica



#endif // PHONOMETRICA_SOUND_PLOT_HPP
