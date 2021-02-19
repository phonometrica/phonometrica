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
 * purpose: custom toolbar toggle button.                                                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TOGGLE_BUTTON_HPP
#define PHONOMETRICA_TOGGLE_BUTTON_HPP

#include <wx/button.h>

namespace phonometrica {

class ToggleButton : public wxButton
{
public:

	ToggleButton(wxWindow *parent, wxWindowID id, const wxBitmap &bmp, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);


	bool IsChecked() const { return m_state; }

	void Check(bool value);

	void Toggle();

	void RestoreBackgroundColour();

protected:

	wxColor m_color_on, m_color_off;

	bool m_state = false;

};

} // namespace phonometrica

#endif // PHONOMETRICA_TOGGLE_BUTTON_HPP
