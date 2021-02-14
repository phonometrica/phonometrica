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
 * Created: 11/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: A toolbar that doesn't suck too much and that provides a consistent behaviour across platforms.            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TOOL_BAR_HPP
#define PHONOMETRICA_TOOL_BAR_HPP

#include <wx/button.h>
#include <wx/tglbtn.h>

namespace phonometrica {

class ToolBar : public wxWindow
{
public:

	explicit ToolBar(wxWindow *parent);

	void AddSeparator();

	void AddSpacer(int space);

	void AddStretchableSpace();

	wxButton *AddButton(const wxBitmap &bitmap, const wxString &tooltip, int id = wxID_ANY);

	wxButton *AddMenuButton(const wxBitmap &bitmap, const wxString &tooltip, int id = wxID_ANY);

	wxToggleButton *AddToggleButton(const wxBitmap &bitmap, const wxString &tooltip, int id = wxID_ANY);

	wxButton *AddHelpButton();

	void ShowMenu(wxButton *button, wxMenu *menu);

protected:

	const wxSize button_size;
};

} // namespace phonometrica



#endif // PHONOMETRICA_TOOL_BAR_HPP
