/***********************************************************************************************************************
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
 * Created: 13/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: The viewer occupies the center of the main window. It is a notebook that can display views as tabs, like   *
 * in a web browser.                                                                                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_VIEWER_HPP
#define PHONOMETRICA_VIEWER_HPP

#include <wx/aui/auibook.h>
#include <phon/runtime.hpp>
#include <phon/gui/views/script_view.hpp>
#ifdef __WXGTK__
#   include <phon/gui/views/linux_start_view.hpp>
#else
#   include <phon/gui/views/start_view.hpp>
#endif

namespace phonometrica {

class MainWindow;

class Viewer final : public wxAuiNotebook
{
public:

	Viewer(Runtime &rt, wxWindow *parent, MainWindow *win);
	void SetStartView();
	void NewScript();
	void NewScript(const String &path);
	void CloseCurrentView();
	View *GetCurrentView();

private:

	void AddView(View *view, const wxString &title);
	void CloseView(int index);
	View *GetView(int i) { return dynamic_cast<View*>(GetPage(i)); }

	// Used to set bindings.
	MainWindow *main_window = nullptr;

	Runtime &runtime;
};

} // namespace phonometrica

#endif // PHONOMETRICA_VIEWER_HPP
