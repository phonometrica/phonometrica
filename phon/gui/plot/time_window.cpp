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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/plot/time_window.hpp>

namespace phonometrica {

TimeAlignedWindow::TimeAlignedWindow(wxWindow *parent) :
	wxWindow(parent, wxID_ANY)
{
	SetBackgroundColour(*wxWHITE);
}

void TimeAlignedWindow::SetTimeWindow(TimeSpan win)
{
	m_window = win;
	InvalidateCache();
	// Don't refresh here: this is done in overrides
}

double TimeAlignedWindow::XPosToTime(double x) const
{
    return m_window.first + (x * GetWindowDuration() / GetWidth());
}

double TimeAlignedWindow::TimeToXPos(double t) const
{
    return (t - m_window.first) * GetWidth() / GetWindowDuration();
}

TimeSpan TimeAlignedWindow::GetTimeWindow() const
{
	return m_window;
}

} // namespace phonometrica
