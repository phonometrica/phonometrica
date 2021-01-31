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

bool View::Finalize(bool autosave)
{
	if (this->IsModified())
	{
		if (autosave)
		{
			this->Save();
		}
		else
		{
			auto reply =  wxMessageBox(_("The current script has unsaved modifications. Would you like to save it?"), _("Save script?"),
			                           wxCANCEL|wxYES|wxNO|wxYES_DEFAULT|wxICON_QUESTION);

			if (reply == wxCANCEL) {
				return false;
			}
			if (reply == wxYES) {
				this->Save();
			}
			else if (reply == wxNO) {
				this->DiscardChanges();
			}
		}
	}
	return true;
}

void View::UpdateTitle()
{
	auto title = GetLabel();

	if (this->IsModified()) {
		title.Append('*');
	}
	SetTitle(title);
}
} // namespace phonometrica