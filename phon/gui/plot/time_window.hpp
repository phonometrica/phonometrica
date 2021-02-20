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
 * Created: 20/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Abstract base class for all windows that represent time-aligned information (sound plots and               *
 * annotation layers.)                                                                                                 *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TIME_WINDOW_HPP
#define PHONOMETRICA_TIME_WINDOW_HPP

#include <wx/window.h>
#include <phon/gui/helpers.hpp>
#include <phon/utils/signal.hpp>

namespace phonometrica {

class TimeAlignedWindow : public wxWindow
{
public:

	TimeAlignedWindow(wxWindow *parent);

	~TimeAlignedWindow() override = default;

	void ChangeWindow(TimeSpan win);

	Signal<TimeSpan> window_changed;

protected:

	virtual void ClearCache() = 0;

	// The current window
	TimeSpan m_window;

};

} // namespace phonometrica



#endif // PHONOMETRICA_TIME_WINDOW_HPP
