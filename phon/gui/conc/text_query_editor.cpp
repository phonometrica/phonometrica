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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/conc/text_query_editor.hpp>

namespace phonometrica {

TextQueryEditor::TextQueryEditor(wxWindow *parent) :
	QueryEditor(parent, _("Query editor")), query(std::make_shared<TextQuery>(nullptr, String()))
{

}

wxWindow *TextQueryEditor::MakeSearchPanel(wxWindow *parent)
{
	return nullptr;
}

AutoQuery TextQueryEditor::GetQuery() const
{
	return query;
}

void TextQueryEditor::ParseQuery()
{
	// FIXME: when to create a new query object?
	assert(query);
	String label = name_ctrl->GetValue();
	if (label != query->label()) {
		query->set_label(label);
	}
	if (!desc_ctrl->IsEmpty())
	{
		auto op = static_cast<DescMetaConstraint::Operator>(desc_op_choice->GetSelection());
		auto mc = std::make_unique<DescMetaConstraint>(op, desc_ctrl->GetValue());
		query->add_metaconstraint(std::move(mc));
	}

	// TODO: files (refactor checkbox)
}
} // namespace phonometrica
