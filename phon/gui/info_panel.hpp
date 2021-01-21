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
 * Created: 13/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: Information panel. This panel is located on the right in the main window and displays information about    *
 * the file(s) that is/are currently selected.                                                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_INFO_PANEL_HPP
#define PHONOMETRICA_INFO_PANEL_HPP

#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/simplebook.h>
#include <phon/runtime.hpp>

namespace phonometrica {

class InfoPanel final : public wxPanel
{
public:

	InfoPanel(Runtime &rt, wxWindow *parent);

private:

	void SetupUi();

	wxSimplebook *m_book;

	Runtime &runtime;
};

} // namespace phonometrica

#endif // PHONOMETRICA_INFO_PANEL_HPP
