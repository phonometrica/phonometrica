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
 * Created: 23/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/cmd/update_match_command.hpp>
#include <phon/gui/views/concordance_view.hpp>

namespace phonometrica {

UpdateMatchCommand::UpdateMatchCommand(ConcordanceView *view, const AutoEvent &event, Match *match) :
	Command("Update match"), m_view(view), m_event(event), m_match(match)
{

}

bool UpdateMatchCommand::execute()
{
	return false;
}

bool UpdateMatchCommand::restore()
{
	return false;
}
} // namespace phonometrica
