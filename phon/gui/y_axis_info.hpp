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
 * Created: 22/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Prepare information on the left of plots and layers in sound and annotation views.                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_Y_AXIS_INFO_HPP
#define PHONOMETRICA_Y_AXIS_INFO_HPP

#include <wx/window.h>
#include <phon/gui/plot/time_window.hpp>
#include <phon/array.hpp>

namespace phonometrica {

class YAxisInfo final : public wxWindow
{
public:

	YAxisInfo(wxWindow *parent);

	void AddWindow(TimeAlignedWindow *win);

	void RemoveWindow(TimeAlignedWindow *win);

	Signal<> invalidate_selection;

private:

	void OnPaint(wxPaintEvent &);

	void OnClick(wxMouseEvent &);

	Array<TimeAlignedWindow*> m_windows;

};

} // namespace phonometrica



#endif // PHONOMETRICA_Y_AXIS_INFO_HPP
