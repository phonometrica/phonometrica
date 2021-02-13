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
 * Purpose: Delete a match in a concordance.                                                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_DELETE_MATCH_COMMAND_HPP
#define PHONOMETRICA_DELETE_MATCH_COMMAND_HPP

#include <wx/cmdproc.h>
#include <phon/application/conc/concordance.hpp>

namespace phonometrica {

class DeleteMatchCommand final : public wxCommand
{
public:

	DeleteMatchCommand(const AutoConcordance &conc, intptr_t row);

	bool Do() override;

	bool Undo() override;

private:

	AutoConcordance m_conc;

	AutoMatch m_match;

	intptr_t m_row;
};

} // namespace phonometrica



#endif // PHONOMETRICA_DELETE_MATCH_COMMAND_HPP
