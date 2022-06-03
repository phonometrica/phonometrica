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
 * Purpose: add a layer in an annotation view.                                                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_ADD_LAYER_COMMAND_HPP
#define PHONOMETRICA_ADD_LAYER_COMMAND_HPP

#include <phon/gui/cmd/command.hpp>

namespace phonometrica {

class AnnotationView;

class AddLayerCommand final : public Command
{
public:

	AddLayerCommand(AnnotationView *view, intptr_t index, const String &label, bool has_instants);

	bool execute() override;

	bool restore() override;

private:

	AnnotationView *m_view;

	intptr_t m_index;

	String m_label;

	bool m_has_instants;
};

} // namespace phonometrica



#endif // PHONOMETRICA_ADD_LAYER_COMMAND_HPP
