/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/toggle_button.hpp>

namespace phonometrica {

ToggleButton::ToggleButton(wxWindow *parent, wxWindowID id, const wxBitmap &bmp, const wxPoint &pos, const wxSize &size) :
	wxButton(parent, id, "", pos, size, wxBORDER_NONE)
{
	SetBitmap(bmp);
	m_color_off = GetBackgroundColour();
	auto r = m_color_off.Red() * 3 / 4;
	auto g = m_color_off.Green() * 3 / 4;
	auto b = m_color_off.Blue() * 3 / 4;
	m_color_on = wxColor(r,g,b);
}

void ToggleButton::Check(bool value)
{
	m_state = value;
	RestoreBackgroundColour();
}

void ToggleButton::Toggle()
{
	Check(!IsChecked());
}

void ToggleButton::RestoreBackgroundColour()
{
	if (m_state)
		SetBackgroundColour(m_color_on);
	else
		SetBackgroundColour(m_color_off);

	Refresh();
}
} // namespace phonometrica