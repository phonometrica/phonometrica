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
 * Created: 06/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Create a dialog based on a user-provided JSON object.                                                      *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_USER_DIALOG_HPP
#define PHONOMETRICA_USER_DIALOG_HPP

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/combobox.h>
#include <wx/textctrl.h>
#include <wx/filepicker.h>
#include <phon/gui/check_list_box.hpp>
#include <phon/gui/sizer.hpp>
#include <phon/runtime/json.hpp>
#include <phon/runtime.hpp>

namespace phonometrica {

class UserDialog final : public wxDialog
{
public:

	UserDialog(wxWindow *parent, Runtime &rt, const Json &js);

	UserDialog(wxWindow *parent, Runtime &rt, const String &str);

	Variant GetJson() const;

private:

	void AddButtons(bool yes_no);

	void OnOk(wxCommandEvent &);

	void OnCancel(wxCommandEvent &);

	bool Parse(const Json &s);

	void ParseItem(Json object);

	String GetName(const Json &item);

	void Add(wxWindow *win);

	void Add(wxSizer *sizer);

	void AddLabel(const Json &item);

	void AddButton(const Json &item);

	void AddCheckBox(const Json &item);

	void AddComboBox(const Json &item);

	void AddLineEdit(const Json &item);

	void AddCheckList(const Json &item);

	void AddRadioButtons(const Json &item);

	void AddFileSelector(const Json &item);

	void AddContainer(const Json &item);

	void AddSpacing(const Json &item);

	wxBoxSizer *current_sizer;

	Dictionary<wxCheckBox*> checkboxes;

	Dictionary<wxComboBox*> comboboxes;

	Dictionary<wxTextCtrl*> fields;

	Dictionary<CheckListBox*> checklists;

	Dictionary<wxRadioBox*> radioboxes;

	Dictionary<wxFilePickerCtrl*> filepickers;

	int sizer_flag = wxEXPAND | wxLEFT | wxTOP | wxRIGHT;

	int sizer_border = 10;

	Runtime &runtime;
};

} // namespace phonometrica



#endif // PHONOMETRICA_USER_DIALOG_HPP
