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
 * Created: 01/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Base class for all query editors.                                                                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_EDITOR_HPP
#define PHONOMETRICA_QUERY_EDITOR_HPP

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <phon/gui/check_list_box.hpp>
#include <phon/gui/property_ctrl.hpp>
#include <phon/application/conc/query.hpp>

namespace phonometrica {

class QueryEditor : public wxDialog
{
public:

	QueryEditor(wxWindow *parent, const wxString &title);

	void Prepare();

	AutoConcordance Execute();

	virtual AutoQuery GetQuery() const = 0;

protected:

	static int GenerateId() { return ++id; }

	wxBoxSizer *MakeHeader(wxWindow *parent);

	virtual wxPanel *MakeSearchPanel(wxWindow *parent) = 0;

	wxWindow *MakeProperties(wxWindow *parent);

	wxBoxSizer *MakeButtons(wxWindow *parent);

	wxBoxSizer *MakeFileSelector(wxWindow *parent);

	void OnOpenHelp(wxCommandEvent &);

	void OnOk(wxCommandEvent &);

	void OnCancel(wxCommandEvent &);

	void OnSave(wxCommandEvent &);

	void OnSaveAs(wxCommandEvent &);

	void SaveQuery(const String &path);

	void OnQueryModified(wxCommandEvent &);

	void EnableSaving(bool value);

	virtual void LoadQuery() = 0;

	virtual void ParseQuery() = 0;

	wxTextCtrl *name_ctrl, *desc_ctrl;

	CheckListBox *file_list;

	wxChoice *desc_op_choice;

	wxButton *save_btn, *save_as_btn;

	Array<PropertyCtrl*> properties;

	bool prepared = false;

	int properties_per_row = 0;

	static int id;
};

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_EDITOR_HPP
