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
 * Created: 15/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Edit the text of an event.                                                                                 *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_EDIT_EVENT_COMMAND_HPP
#define PHONOMETRICA_EDIT_EVENT_COMMAND_HPP

#include <phon/gui/cmd/command.hpp>
#include <phon/application/annotation.hpp>

namespace phonometrica {

class EditEventCommand final : public Command
{
public:

	EditEventCommand(const Handle<Annotation> &annot, const AutoEvent &event, const String &new_value);

	bool execute() override;

	bool restore() override;

private:

	bool change_value();

	Handle<Annotation> m_annot;

	AutoEvent m_event;

	String value;
};

} // namespace phonometrica



#endif // PHONOMETRICA_EDIT_EVENT_COMMAND_HPP
