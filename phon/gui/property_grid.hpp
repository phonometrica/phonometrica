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
 * Created: 26/01/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Property editor in the information panel.                                                                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PROPERTY_GRID_HPP
#define PHONOMETRICA_PROPERTY_GRID_HPP

#include <wx/grid.h>
#include <phon/application/property.hpp>

namespace phonometrica {

class PropertyGrid final : public wxGrid
{
public:

	PropertyGrid(wxWindow *parent);

	void AppendProperties(const std::set<Property> &properties);

	void SetEditingMode(int row, bool value);

	void SetPropertyEditor(int row, const String &category);

private:

	void OnResize(wxSizeEvent &e);

};

} // namespace phonometrica

#endif // PHONOMETRICA_PROPERTY_GRID_HPP
