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
 * Created: 11/06/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/cmd/add_layer_command.hpp>
#include <phon/gui/views/annotation_view.hpp>

namespace phonometrica {

AddLayerCommand::AddLayerCommand(AnnotationView *view, intptr_t index, const String &label, bool has_instants)
		: Command("Add layer", true), m_view(view), m_index(index), m_label(label), m_has_instants(has_instants)
{

}

bool AddLayerCommand::execute()
{
	try {
		m_view->AddLayer(m_index, m_label, m_has_instants);
		return true;
	}
	catch (...) {
		return false;
	}
}

bool AddLayerCommand::restore()
{
	try {
		m_view->RemoveLayer(m_index);
		return true;
	}
	catch (...) {
		return false;
	}
}
} // namespace phonometrica
