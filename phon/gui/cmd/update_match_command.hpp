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
 * Purpose: Update a match in a concordance when the corresponding event was edited.                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_UPDATE_MATCH_COMMAND_HPP
#define PHONOMETRICA_UPDATE_MATCH_COMMAND_HPP

#include <phon/gui/cmd/command.hpp>
#include <phon/application/conc/match.hpp>

namespace phonometrica {

class ConcordanceView;


class UpdateMatchCommand : public Command
{
public:

	UpdateMatchCommand(ConcordanceView *view, const AutoEvent &event, Match *match);

	bool execute() override;

	bool restore() override;

private:

	ConcordanceView *m_view;

	AutoEvent m_event;

	// This pointer is valid during the lifetime of the command.
	Match *m_match;
};

} // namespace phonometrica



#endif // PHONOMETRICA_UPDATE_MATCH_COMMAND_HPP
