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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/cmd/edit_event_command.hpp>

namespace phonometrica {

EditEventCommand::EditEventCommand(const Handle<Annotation> &annot, const AutoEvent &event, const String &new_value) :
	Command("Edit event", true), m_annot(annot), m_event(event), value(new_value)
{

}

bool EditEventCommand::execute()
{
	return change_value();
}

bool EditEventCommand::restore()
{
	return change_value();
}

bool EditEventCommand::change_value()
{
	auto previous_value = m_event->text();
	m_annot->set_event_text(m_event, this->value);
	this->value = previous_value;

	return true;
}
} // namespace phonometrica
