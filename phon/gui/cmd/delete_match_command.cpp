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
 * Created: 13/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/msgdlg.h>
#include <phon/gui/cmd/delete_match_command.hpp>
#include <phon/gui/views/concordance_view.hpp>

namespace phonometrica {

DeleteMatchCommand::DeleteMatchCommand(ConcordanceView *view, const Handle <Concordance> &conc, intptr_t row) :
	Command(_("Delete match"), true), m_view(view), m_conc(conc), m_row(row)
{

}

bool DeleteMatchCommand::execute()
{
	if (this->next)
	{
		if (!this->next->execute()) {
			return false;
		}
	}

	return do_execute();
}

bool DeleteMatchCommand::restore()
{
	if (!do_restore()) {
		return false;
	}

	if (this->next)
	{
		if (!this->next->restore()) {
			return false;
		}
	}

	return true;
}

bool DeleteMatchCommand::do_execute()
{
	try
	{
		m_match = m_conc->remove_match(m_row);
		m_view->DeleteRow((int)m_row - 1);

		return true;
	}
	catch (std::exception &e)
	{
		auto msg = wxString::Format(_("Could not delete row %d: %s"), (int)m_row, e.what());
		wxMessageBox(msg, _("Error"), wxICON_ERROR);

		return false;
	}
}

bool DeleteMatchCommand::do_restore()
{
	try
	{
		m_conc->restore_match(m_row, std::move(m_match));
		m_view->RestoreRow((int)m_row - 1);

		return true;
	}
	catch (std::exception &e)
	{
		auto msg = wxString::Format(_("Could not restore row %d: %s"), (int)m_row, e.what());
		wxMessageBox(msg, _("Error"), wxICON_ERROR);
		return false;
	}
}
} // namespace phonometrica
