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
 * Purpose: Abstract base class for all windows that represent time-aligned information (sound plots and               *
 * annotation layers.)                                                                                                 *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TIME_WINDOW_HPP
#define PHONOMETRICA_TIME_WINDOW_HPP

#include <wx/window.h>
#ifdef __WXMSW__
#include <wx/dcbuffer.h>
#else
#include <wx/dcclient.h>
#endif

#include <phon/gui/helpers.hpp>
#include <phon/utils/signal.hpp>



namespace phonometrica {

// We need this on Windows to avoid flickering on the axes
#ifdef __WXMSW__
typedef wxBufferedPaintDC PaintDC;
#else
typedef wxPaintDC PaintDC;
#endif


class TimeAlignedWindow : public wxWindow
{
public:

	TimeAlignedWindow(wxWindow *parent);

	~TimeAlignedWindow() override = default;

	TimeWindow GetTimeWindow() const;

	void SetTimeWindow(TimeWindow win);

	virtual void DrawYAxis(PaintDC &dc, const wxRect &rect) = 0;

	Signal<TimeWindow> update_window;

protected:

	friend class SoundView;

	double GetWindowDuration() const { return m_window.second - m_window.first; }

	int GetWidth() const { return GetSize().GetWidth(); }

	int GetHeight() const { return GetSize().GetHeight(); }

	double XPosToTime(double x) const;

	double TimeToXPos(double t) const;

	virtual void InvalidateCache() = 0;

	// The current window
	TimeWindow m_window = {-1.0, -1.0};
};

} // namespace phonometrica



#endif // PHONOMETRICA_TIME_WINDOW_HPP
