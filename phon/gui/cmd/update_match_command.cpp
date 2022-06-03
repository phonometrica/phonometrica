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
 * Created: 23/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/msgdlg.h>
#include <phon/gui/cmd/update_match_command.hpp>
#include <phon/gui/views/concordance_view.hpp>

namespace phonometrica {

UpdateMatchCommand::UpdateMatchCommand(ConcordanceView *view, const Handle <Concordance> &conc, intptr_t match, intptr_t target) :
	Command("Update match"), m_view(view), m_conc(conc), m_match(match), m_target(target)
{

}

bool UpdateMatchCommand::execute()
{
	auto result = m_conc->update_match(m_match, m_target);

	if (result)
	{
		update_matches();
	}
	else
	{
		wxMessageBox(_("The current match is no longer valid and will be deleted"),
		             _("Invalid match"), wxICON_WARNING);
		m_view->DeleteRow(m_match, true);
	}

	return result;
}

bool UpdateMatchCommand::restore()
{
	auto result = m_conc->update_match(m_match, m_target);

	if (result)
	{
		update_matches();
	}
	else
	{
		wxMessageBox(_("The current match could not be restored"), _("Error"), wxICON_ERROR);
	}
	m_view->ClearSelection();
	m_view->SelectRow(m_match - 1);

	return result;
}

void UpdateMatchCommand::update_matches()
{
	// We need to update the grid since the edited match might appear in the context of another match.
	// We don't want to update the whole grid, so we choose a reasonable range around the edited item.
	const int span = 10;
	auto first_match = (std::max<intptr_t>)(1, m_match - span) - 1;
	auto last_match = (std::min<intptr_t>)(m_match + span, m_conc->row_count()) - 1;

	for (intptr_t i = first_match; i <= last_match; i++)
	{
		m_conc->update_context(i+1);
		m_view->UpdateRow(i);
	}
}
} // namespace phonometrica
