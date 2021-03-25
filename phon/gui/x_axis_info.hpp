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
 * Purpose: Widget to display time information between the toolbar and the plots in annotation and sound views.        *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_X_AXIS_INFO_HPP
#define PHONOMETRICA_X_AXIS_INFO_HPP

#include <wx/dcclient.h>
#include <wx/window.h>
#include <phon/gui/helpers.hpp>
#include <phon/utils/signal.hpp>

namespace phonometrica {

class XAxisInfo final : public wxWindow
{
public:

	XAxisInfo(wxWindow *parent);

	void SetTimeWindow(TimeWindow win);

	void SetSelection(const TimeSelection &sel);

	void InvalidateSelection() { m_sel.invalidate(); Refresh(); }

	Signal<> invalidate_selection;

private:

	bool HasTimeWindow() const { return m_win.first >= 0; }

	bool HasSelection() const { return m_sel.t1 >= m_win.first && m_sel.t2 <= m_win.second; }

	bool HasPointSelection() const { return m_sel.is_point(); }

	void OnPaint(wxPaintEvent &e);

	double TimeToXPos(double t) const;

	void OnClick(wxMouseEvent &);

	TimeWindow m_win = {-1., -1.};

	TimeSelection m_sel = {-1, -1};
};

} // namespace phonometrica



#endif // PHONOMETRICA_X_AXIS_INFO_HPP
