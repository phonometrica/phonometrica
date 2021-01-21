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
 * Created: 19/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: Base class for all preferences dialog.                                                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PREFERENCES_DIALOG_HPP
#define PHONOMETRICA_PREFERENCES_DIALOG_HPP

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/notebook.h>

namespace phonometrica {

class PreferencesDialog : public wxDialog
{
public:

	PreferencesDialog(wxWindow *parent, const wxString &title);

protected:

	void AddPage(wxWindow *page, const wxString &title);
	void CreateButtons();
	void OnReset(wxCommandEvent &);
	void OnOk(wxCommandEvent &);
	void OnCancel(wxCommandEvent &);

	// Override these functions in subclasses to implement custom behavior. These will be called
	// automatically when the buttons are clicked.
	virtual void DoReset() = 0;
	virtual void DoOk() = 0;
	virtual void DoCancel() { }

	wxNotebook *m_book;

	wxBoxSizer *m_sizer;

};

} // namespace phonometrica

#endif // PHONOMETRICA_PREFERENCES_DIALOG_HPP
