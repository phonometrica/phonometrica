/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 24/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: flag to track the mouse in speech views.                                                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_MOUSE_TRACKING_HPP
#define PHONOMETRICA_MOUSE_TRACKING_HPP

namespace phonometrica {

enum class MouseTracking
{
	Disabled = 0,   // don't track mouse in speech plot
	Enabled,        // always track mouse
	Anchored        // temporarily track mouse while an anchor is being moved
};

} // namespace phonometrica

#endif // PHONOMETRICA_MOUSE_TRACKING_HPP
