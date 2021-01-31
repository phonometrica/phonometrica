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
	Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &Viewer::OnCloseView, this);
	Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSED, &Viewer::OnViewClosed, this);
}

void Viewer::NewScript()
{
	auto view = new ScriptView(runtime, std::make_shared<Script>(nullptr), this);
	AddView(view, _("Untitled script"));
}

void Viewer::AddView(View *view, const wxString &title)
{
	AddPage(view, title, true);
}

void Viewer::NewScript(const AutoScript &script)
{
	AddView(new ScriptView(runtime, script, this), script->label());
}

void Viewer::CloseCurrentView()
{
	CloseView(GetSelection(), true);
}

void Viewer::CloseView(int index, bool remove)
{
	GetView(index)->Close();
	if (remove) {
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

void Viewer::OnViewFile(const std::shared_ptr<VFile> &file)
{
	for (size_t i = 0; i < GetPageCount(); i++)
	{
		if (GetView(i)->path() == file->path())
		{
			SetSelection(i);
			return;
		}
	}

	if (file->is_script())
	{
		NewScript(downcast<Script>(file));
	}
}

void Viewer::AdjustFontSize()
{
	for (size_t i = 0; i < GetPageCount(); i++)
	{
		GetView(i)->AdjustFontSize();
	}
}

void Viewer::OnCloseView(wxAuiNotebookEvent &e)
{
	if (!GetCurrentView()->Finalize(false)) {
		e.Veto();
	}
}

void Viewer::OnViewClosed(wxAuiNotebookEvent &)
{
	SetStartView();
}

bool Viewer::SaveViews(bool autosave)
{
	bool result = true;

	for (size_t i = 0; i < GetPageCount(); i++)
	{
		auto view = GetView(i);

		// In autosave mode, we save everything without switching to each view
		if (view->IsModified() && !autosave) {
			SetSelection(i);
		}
		if (!view->Finalize(autosave)) {
			result = false;
		}
	}

	return result;
}

} // namespace phonometrica