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
 * Created: 28/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: Import/export CSV files.                                                                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CSV_DIALOG_HPP
#define PHONOMETRICA_CSV_DIALOG_HPP

#include <wx/dialog.h>
#include <wx/filepicker.h>
#include <wx/choice.h>
#include <wx/radiobox.h>
#include <phon/string.hpp>

namespace phonometrica {

class CsvDialog final : public wxDialog
{
public:

	CsvDialog(wxWindow *parent, const wxString &title, bool read);

	String GetPath() const;

	String GetSeparator() const;

private:

	void OnOk(wxCommandEvent &);

	void OnCancel(wxCommandEvent &);

	wxFilePickerCtrl *file_picker;

	wxRadioBox *sep_box;
};

} // namespace phonometrica

#endif // PHONOMETRICA_CSV_DIALOG_HPP
