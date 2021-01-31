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
 * Created: 29/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: File and directory dialogs.                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_DIALOG_HPP
#define PHONOMETRICA_DIALOG_HPP

#if PHON_WINDOWS
// wxWidgets includes an old header, winsock.h, wich defines symbols
// later redefined by winsock2.h. We include that file before including
// any wxWidgets header to avoid compiler errors.
#include <WinSock2.h>
#include <wx/msw/winundef.h>
#endif
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>

namespace phonometrica {

class FileDialog final : public wxFileDialog
{
public:

	FileDialog(wxWindow *parent, const wxString &message, const wxString &default_file = wxEmptyString,
			const wxString &wildcard = wxString::FromAscii(wxFileSelectorDefaultWildcardStr), long style = wxFD_OPEN);

	wxString GetPath() const override;

	void GetPaths(wxArrayString &paths) const override;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class DirDialog final : public wxDirDialog
{
public:

	DirDialog(wxWindow *parent, const wxString &message, long style = wxDD_DEFAULT_STYLE);

	wxString GetPath() const override;

	void GetPaths(wxArrayString &paths) const override;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline
int ask_question(const wxString &message, const wxString &caption)
{
	return wxMessageBox(message, caption, wxYES|wxNO|wxNO_DEFAULT|wxICON_QUESTION);
}

} // namespace phonometrica

#endif // PHONOMETRICA_DIALOG_HPP
