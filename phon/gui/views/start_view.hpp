/***********************************************************************************************************************
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
 * Created: 14/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: Start view.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_START_VIEW_HPP
#define PHONOMETRICA_START_VIEW_HPP

#include <wx/button.h>
#include <wx/sizer.h>
#include <phon/gui/views/view.hpp>

#ifndef __WXGTK__

namespace phonometrica {

class MainWindow;

class StartView final : public View
{
public:

	explicit StartView(wxWindow *parent, MainWindow *win);
	bool Finalize() override { return true; }

private:

	wxButton *MakeButton(std::string_view filename, const wxString &description);
};


} // namespace phonometrica

#endif // __WXGTK__
#endif // PHONOMETRICA_START_VIEW_HPP
