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

#include <phon/gui/views/view.hpp>
#include <phon/gui/viewer.hpp>
#include <phon/gui/dialog.hpp>
#include <phon/application/project.hpp>

namespace phonometrica {

View::View(wxWindow *parent) : wxPanel(parent)
{

}

void View::SetTitle(const wxString &title)
{
	auto viewer = dynamic_cast<Viewer*>(GetParent());
	viewer->SetPageText(viewer->GetSelection(), title);
}

wxString View::GetTitle() const
{
	auto viewer = dynamic_cast<Viewer*>(GetParent());
	return viewer->GetPageText(viewer->GetSelection());
}

void View::MakeTitleModified()
{
	auto title = GetTitle();
	if (!title.EndsWith("*"))
	{
		title.Append('*');
		SetTitle(title);
	}
}

void View::MakeTitleUnmodified()
{
	auto title = GetTitle();
	if (title.EndsWith("*")) {
		SetTitle(title.Left(title.size() - 1));
	}
}

void View::AskImportFile(const String &path)
{
	auto reply = ask_question(_("Would you like to import this annotation into the current project?"), _("Import file?"));

	if (reply == wxYES)
	{
		auto project = Project::get();
		project->import_file(path);
		project->notify_update();
	}
}

} // namespace phonometrica