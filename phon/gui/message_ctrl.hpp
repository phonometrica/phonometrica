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
 * Created: 07/04/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Display messages in sound views when the cursor is moved (e.g. waveform amplitude, pitch, intensity, etc.).*
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_MESSAGE_CTRL_HPP
#define PHONOMETRICA_MESSAGE_CTRL_HPP

#include <wx/window.h>
#include <wx/stattext.h>

namespace phonometrica {

class MessageCtrl final : public wxWindow
{
public:

	MessageCtrl(wxWindow *parent);

	void Print(const wxString &text);

	void Clear();

protected:

	wxStaticText *text_ctrl;
};

} // namespace phonometrica



#endif // PHONOMETRICA_MESSAGE_CTRL_HPP
