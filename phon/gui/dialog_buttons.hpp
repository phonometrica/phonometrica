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
 * Created: 11/06/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: OK/Cancel buttons for dialogs.                                                                             *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_DIALOG_BUTTONS_HPP
#define PHONOMETRICA_DIALOG_BUTTONS_HPP

#include <wx/sizer.h>

namespace phonometrica {

class DialogButtons : public wxBoxSizer
{
public:

	DialogButtons(wxWindow *parent);

};

} // namespace phonometrica



#endif // PHONOMETRICA_DIALOG_BUTTONS_HPP
