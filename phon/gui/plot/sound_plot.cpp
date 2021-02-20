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

#include <phon/gui/plot/sound_plot.hpp>

namespace phonometrica {

SoundPlot::SoundPlot(wxWindow *parent, const Handle<Sound> &snd) :
	TimeAlignedWindow(parent), m_sound(snd)
{
	Bind(wxEVT_LEFT_DOWN, &SoundPlot::OnStartSelection, this);
	Bind(wxEVT_LEFT_UP, &SoundPlot::OnEndSelection, this);
	Bind(wxEVT_MOTION, &SoundPlot::OnMotion, this);
	Bind(wxEVT_LEAVE_WINDOW, &SoundPlot::OnLeaveWindow, this);
}

void SoundPlot::OnStartSelection(wxMouseEvent &e)
{
	auto pos = e.GetPosition();
	m_sel_start = pos.x;
	e.Skip();
}

void SoundPlot::OnEndSelection(wxMouseEvent &e)
{
	m_sel_start = -1;
	e.Skip();
}

void SoundPlot::OnMotion(wxMouseEvent &e)
{
	if (m_sel_start >= 0)
	{
		auto pos = e.GetPosition();
		double x = pos.x;

		// We don't need to refresh the plot because the signal will be sent back to us by the view
		// to which this plot is connected.
		if (x < m_sel_start) {
			update_selection(PixelSelection{x, m_sel_start});
		}
		else {
			update_selection(PixelSelection{m_sel_start, x});
		}
	}
}

void SoundPlot::OnLeaveWindow(wxMouseEvent &e)
{
	auto pos = e.GetPosition();
	m_sel_start = -1;
	e.Skip();
}

void SoundPlot::OnSelectionChanged(PixelSelection sel)
{
	m_sel = sel;
	Refresh();
}

} // namespace phonometrica
