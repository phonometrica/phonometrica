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
 * Purpose: Abstract base class for all commands in Phonometrica: a command knows how to do and undo itself, in such   *
 * a way that undoing the command restores the state of the program before applying the command.                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_COMMAND_HPP
#define PHONOMETRICA_COMMAND_HPP

#include <phon/string.hpp>

namespace phonometrica {

class Command
{
public:

	explicit Command(String name, bool undo = true);

	// Returns true if the command was successfully performed, and false otherwise
	virtual bool execute() = 0;

	// Returs true if the command was successfully undone, and false otherwise
	virtual bool restore() = 0;

	const String &name() const;

	bool can_undo() const;

protected:

	String m_name;

	bool m_can_undo;
};


using AutoCommand = std::unique_ptr<Command>;

} // namespace phonometrica



#endif // PHONOMETRICA_COMMAND_HPP
