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
 * Created: 14/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_VIEW_HPP
#define PHONOMETRICA_VIEW_HPP

#include <wx/panel.h>

namespace phonometrica {

class View : public wxPanel
{
public:

	explicit View(wxWindow *parent);

	// This method is called before the view is destroyed. It returns true
	// if the view can be closed, false if it must be kept open.
	virtual bool Finalize() = 0;

	// These methods can be overriden to respond to accelerators such as ctrl+s (save) or ctrl+r (run)
	virtual void Save() { }
	virtual void Run() { }

protected:

	// These functions assume that the view is selected.
	wxString GetTitle() const;
	void SetTitle(const wxString &title);
	void MakeTitleModified();
	void MakeTitleUnmodified();

};

} // namespace phonometrica

#endif // PHONOMETRICA_VIEW_HPP
