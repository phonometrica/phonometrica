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
 * Purpose: Update a match in a concordance when the corresponding event was edited.                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_UPDATE_MATCH_COMMAND_HPP
#define PHONOMETRICA_UPDATE_MATCH_COMMAND_HPP

#include <phon/gui/cmd/command.hpp>
#include <phon/application/conc/concordance.hpp>

namespace phonometrica {

class ConcordanceView;


class UpdateMatchCommand final : public Command
{
public:

	UpdateMatchCommand(ConcordanceView *view, const Handle <Concordance> &conc, intptr_t match, intptr_t target);

	bool execute() override;

	bool restore() override;

private:

	void update_matches();

	ConcordanceView *m_view;

	const Handle<Concordance> m_conc;

	intptr_t m_match, m_target;
};

} // namespace phonometrica



#endif // PHONOMETRICA_UPDATE_MATCH_COMMAND_HPP
