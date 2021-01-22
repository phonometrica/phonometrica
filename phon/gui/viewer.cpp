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
 * Created: 13/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/viewer.hpp>
#include <phon/gui/tab_art_provider.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Viewer::Viewer(Runtime &rt, wxWindow *parent, MainWindow *win) :
	wxAuiNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), runtime(rt)
{
	main_window = win;
	SetArtProvider(new TabArtProvider());
}

void Viewer::NewScript()
{
	AddView(new ScriptView(runtime, this), _("Untitled script"));
}

void Viewer::AddView(View *view, const wxString &title)
{
	AddPage(view, title, true);
}

void Viewer::NewScript(const String &path)
{
	AddView(new ScriptView(runtime, path, this), filesystem::base_name(path));
}

void Viewer::CloseCurrentView()
{
	CloseView(GetSelection());
	SetStartView();
}

void Viewer::CloseView(int index)
{
	auto view = GetView(index);

	if (view->Finalize()) {
		DeletePage(index);
	}
}

void Viewer::SetStartView()
{
	if (GetPageCount() == 0)
	{
		auto view = new StartView(this, main_window);
		AddPage(view, _("Start"));
	}
}

View *Viewer::GetCurrentView()
{
	return GetView(GetSelection());
}
} // namespace phonometrica