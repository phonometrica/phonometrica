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
 * Created: 13/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/msgdlg.h>
#include <phon/gui/cmd/command_processor.hpp>

namespace phonometrica {

CommandProcessor::CommandProcessor(size_t limit) :
		m_limit(limit)
{

}

void CommandProcessor::submit(AutoCommand cmd)
{
	if (cmd->execute())
	{
		m_commands.resize(m_pos);
		m_commands.push_back(std::move(cmd));
		m_pos++;

		if ((ssize_t)m_commands.size() > m_limit) {
			m_commands.pop_front();
		}
	}
}

void CommandProcessor::undo()
{
	if (m_commands.empty() || m_pos <= 0)
	{
		wxMessageBox(_("Nothing to undo in the current view!"), _("Invalid edit operation"), wxICON_INFORMATION);
	}
	else
	{
		m_commands[--m_pos]->restore();
	}
}

void CommandProcessor::redo()
{
	if (m_commands.empty() || m_pos >= (ssize_t) m_commands.size())
	{
		wxMessageBox(_("Nothing to redo in the current view!"), _("Invalid edit operation"), wxICON_INFORMATION);
	}
	else
	{
		m_commands[m_pos++]->execute();
	}
}

} // namespace phonometrica
