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
 * Purpose: Thin lines to separate widgets.                                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_LINES_HPP
#define PHONOMETRICA_LINES_HPP

#include <wx/window.h>
#include <phon/gui/helpers.hpp>

namespace phonometrica {

class HLine final : public wxWindow
{
public:

	HLine(wxWindow *parent) : wxWindow(parent, wxID_ANY)
	{
		SetSize(wxSize(-1, 1));
		SetBackgroundColour(LINE_COLOUR);
	}

};

class VLine final : public wxWindow
{
public:

	VLine(wxWindow *parent) : wxWindow(parent, wxID_ANY)
	{
		SetSize(wxSize(1, 30));
		SetBackgroundColour(LINE_COLOUR);
	}

};

} // namespace phonometrica



#endif // PHONOMETRICA_LINES_HPP
