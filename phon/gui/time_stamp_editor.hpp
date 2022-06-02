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
 * Created: 24/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Bookmark editor for time stamps                                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TIME_STAMP_EDITOR_HPP
#define PHONOMETRICA_TIME_STAMP_EDITOR_HPP

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <phon/application/bookmark.hpp>

namespace phonometrica {

class TimeStampEditor final : public wxDialog
{
public:

	TimeStampEditor(wxWindow *parent);

	wxString GetBookmarkTitle() const;

	wxString GetNotes() const;

private:

	wxTextCtrl *title_ctrl, *notes_ctrl;

};

} // namespace phonometrica



#endif // PHONOMETRICA_TIME_STAMP_EDITOR_HPP
