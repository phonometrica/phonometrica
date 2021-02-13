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

#include <wx/msgdlg.h>
#include <phon/gui/viewer.hpp>
#include <phon/gui/views/concordance_view.hpp>
#include <phon/gui/tab_art_provider.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/application/project.hpp>

namespace phonometrica {

Viewer::Viewer(Runtime &rt, wxWindow *parent, MainWindow *win) :
	wxAuiNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), cmd_proc(100), runtime(rt)
{
	main_window = win;
	SetArtProvider(new TabArtProvider());
	Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &Viewer::OnCloseView, this);
	Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSED, &Viewer::OnViewClosed, this);
	Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &Viewer::OnPageChanged, this);
}

void Viewer::NewScript()
{
	NewScriptWithParent(nullptr);
}

void Viewer::NewScriptWithParent(VFolder *parent)
{
	if (!parent) {
		parent = Project::get()->scripts().get();
	}
	auto script = std::make_shared<Script>(parent);
	auto view = new ScriptView(runtime, script, this);
	AddView(view, _("Untitled script"));
	// Don't mutate the folder because this is an empty script
	parent->append(std::move(script), false);
	Project::updated();
}

void Viewer::AddView(View *view, const wxString &title)
{
	view->SetCommandProcessor(&cmd_proc);
	AddPage(view, title, true);
}

void Viewer::NewScript(const AutoScript &script)
{
	auto view = new ScriptView(runtime, script, this);
	AddView(view, script->label());
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

void Viewer::ViewFile(const std::shared_ptr<VFile> &file)
{
	file->open();

	for (size_t i = 0; i < GetPageCount(); i++)
	{
		auto path = file->path();
		if (!path.empty() && GetView(i)->GetPath() == path)
		{
			SetSelection(i);
			return;
		}
	}

	if (file->is_script())
	{
		NewScript(downcast<Script>(file));
	}
	else if (file->is_concordance())
	{
		auto conc = downcast<Concordance>(file);

		if (conc->empty())
		{
			wxMessageBox(_("No match found!"), _("Empty concordance"), wxICON_INFORMATION);
		}
		else
		{
			auto label = conc->label();
			AddView(new ConcordanceView(this, std::move(conc)), label);
		}
	}
	else
	{
		wxMessageBox(_("Not implemented yet!"), _("Information"), wxICON_INFORMATION);
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

void Viewer::OnPageChanged(wxAuiNotebookEvent &e)
{
	int sel = e.GetSelection();
	SetPageToolTip(sel, GetView(sel)->GetPath());
}

void Viewer::UpdateCurrentView()
{
	GetCurrentView()->Refresh();
}

void Viewer::SetupCommandProcessor(wxMenu *menu)
{
	cmd_proc.SetEditMenu(menu);
//	cmd_proc.SetUndoAccelerator("Undo\tctrl+z");
//	cmd_proc.SetRedoAccelerator("Redo\tctrl+y");
	cmd_proc.Initialize();
}

} // namespace phonometrica