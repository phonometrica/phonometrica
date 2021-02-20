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
#include <phon/gui/views/sound_view.hpp>
#include <phon/gui/viewer.hpp>
#include <phon/gui/views/concordance_view.hpp>
#include <phon/gui/tab_art_provider.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/application/cmd/edit_event_command.hpp>
#include <phon/application/project.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

Viewer::Viewer(Runtime &rt, wxWindow *parent, MainWindow *win) :
	wxAuiNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), runtime(rt)
{
	main_window = win;
	SetArtProvider(new TabArtProvider());
	Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &Viewer::OnCloseView, this);
	Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSED, &Viewer::OnViewClosed, this);
	Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &Viewer::OnPageChanged, this);
	Annotation::edit_event.connect(&Viewer::OnEditEvent, this);
}

void Viewer::NewScript()
{
	NewScriptWithParent(nullptr);
}

void Viewer::NewScriptWithParent(Directory *parent)
{
	if (!parent) {
		parent = Project::get()->scripts().get();
	}
	auto script = make_handle<Script>(parent);
	auto view = new ScriptView(runtime, script, this);
	AddView(view, _("Untitled script"));
	// Don't mutate the folder because this is an empty script
	parent->append(std::move(script), false);
	Project::updated();
}

void Viewer::AddView(View *view, const wxString &title)
{
	AddPage(view, title, true);
}

void Viewer::NewScript(const Handle<Script> &script)
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

void Viewer::ViewFile(const Handle<Document> &file)
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

	if (file->is<Sound>())
	{
		auto snd = recast<Sound>(file);
		auto view = new SoundView(this, snd);
		view->Initialize();
		AddView(view, snd->label());
	}
	else if (file->is<Script>())
	{
		NewScript(recast<Script>(file));
	}
	else if (file->is<Concordance>())
	{
		auto conc = recast<Concordance>(file);

		if (conc->empty() && Settings::get_boolean("concordance", "discard_empty"))
		{
			wxMessageBox(_("No match found!"), _("Empty concordance"), wxICON_INFORMATION);
		}
		else
		{
		    wake_up();
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

void Viewer::CloseViews()
{
	SaveViews(Settings::get_boolean("autosave"));
	for (size_t i = GetPageCount(); i-- > 0; )
	{
		auto view = GetView(i);
		if (!view->IsStartView())
		{
			RemovePage(i);
			delete view;
		}
	}
	SetStartView();
}

void Viewer::OnEditEvent(const Handle<Annotation> &annot, const AutoEvent &event, const String &new_value)
{
	auto cmd = std::make_unique<EditEventCommand>(annot, event, new_value);
	GetCurrentView()->Submit(std::move(cmd));
}

void Viewer::UpdateLabels()
{
	for (size_t i = 0; i < GetPageCount(); i++) {
		GetView(i)->UpdateTitle();
	}
}

} // namespace phonometrica