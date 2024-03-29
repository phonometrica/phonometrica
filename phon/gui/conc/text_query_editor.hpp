/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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
 * Purpose: Editor for text get_concordances in annotations.                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TEXT_QUERY_EDITOR_HPP
#define PHONOMETRICA_TEXT_QUERY_EDITOR_HPP

#include <wx/radiobox.h>
#include <wx/spinctrl.h>
#include <phon/gui/conc/query_editor.hpp>
#include <phon/gui/conc/constraint_ctrl.hpp>
#include <phon/application/conc/query.hpp>

class wxRadioButton;

namespace phonometrica {

class TextQueryEditor final : public QueryEditor
{
public:

	explicit TextQueryEditor(wxWindow *parent);

	explicit TextQueryEditor(wxWindow *parent, Handle<Query> q);

	Handle<Query> GetQuery() const override;

	void LoadQuery() override;

private:

	wxPanel *MakeSearchPanel(wxWindow *parent) override;

	void ParseQuery() override;

	void OnAddConstraint(wxCommandEvent &);

	void OnRemoveConstraint(wxCommandEvent &);

	void OnSearch(wxCommandEvent &e);

	void OnEnableSaving(wxCommandEvent &);

	wxBoxSizer *constraint_sizer;

	wxButton *add_constraint_btn, *remove_constraint_btn;

	wxStaticBox *constraint_box;

	wxRadioButton *ctx_btn1, *ctx_btn2, *ctx_btn3;

	wxSpinCtrl *context_spinctrl, *ref_spinctrl;

	Handle<Query> query;

	Array<ConstraintCtrl*> constraints;
};

} // namespace phonometrica



#endif // PHONOMETRICA_TEXT_QUERY_EDITOR_HPP
