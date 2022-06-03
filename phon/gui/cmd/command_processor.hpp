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
 * Purpose: Manage a queue of commands.                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_COMMAND_PROCESSOR_HPP
#define PHONOMETRICA_COMMAND_PROCESSOR_HPP

#include <deque>
#include <phon/gui/cmd/command.hpp>

namespace phonometrica {

class CommandProcessor final
{
public:

	explicit CommandProcessor(size_t limit = 50);

	bool submit(AutoCommand cmd);

	void undo();

	void redo();

private:

	std::deque<AutoCommand> m_commands;

	ssize_t m_limit;

	ssize_t m_pos = 0;
};

} // namespace phonometrica



#endif // PHONOMETRICA_COMMAND_PROCESSOR_HPP
