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
 * Created: 03/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Prepare a search constraint in the query editor.                                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CONSTRAINT_CTRL_HPP
#define PHONOMETRICA_CONSTRAINT_CTRL_HPP

#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <phon/application/conc/constraint.hpp>

namespace phonometrica {

struct ConstraintCtrl final : public wxPanel
{
	using Operator = Constraint::Operator;

	explicit ConstraintCtrl(wxWindow *parent, int index, bool enable_relation);

	void EnableRelation(bool value);

	Operator GetSearchOperator() const;

	bool IsCaseSensitive() const;

	Constraint ParseConstraint() const;

	void LoadConstraint(const Constraint &constraint);

	// Displays the layer index or name pattern
	wxTextCtrl *layer_ctrl;

	// Search field
	wxSearchCtrl *search_ctrl;

	// Select relation to next constraint (in case of a complex query)
	wxChoice *relation_selector;

	// Search operator
	wxChoice *operator_selector;

	// Case
	wxCheckBox *case_checkbox;

private:

	void SetLayerDescriptiveText(bool focus);

	void SetLayerText(const wxString &text);
};

} // namespace phonometrica

#endif // PHONOMETRICA_CONSTRAINT_CTRL_HPP
