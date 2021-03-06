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

#include <phon/gui/cmd/command.hpp>

namespace phonometrica {

phonometrica::Command::Command(String name, bool undo) : m_name(std::move(name)), m_can_undo(undo)
{

}

const String &Command::name() const
{
	return m_name;
}

bool Command::can_undo() const
{
	return m_can_undo;
}

void Command::append(std::unique_ptr<Command> cmd)
{
	auto current = this;

	while (current->next) {
		current = current->next.get();
	}
	current->next = std::move(cmd);
}

} // namespace phonometrica
