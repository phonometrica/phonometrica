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

#include <wx/sizer.h>
#include <wx/artprov.h>
#include <wx/msgdlg.h>
#include <phon/gui/views/script_view.hpp>
#include <phon/application/macros.hpp>
#include <phon/gui/console.hpp>
#include <phon/gui/dialog.hpp>
#include <phon/gui/text_viewer.hpp>
#include <phon/include/icons.hpp>
#include <phon/file.hpp>
#include <phon/application/settings.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/application/project.hpp>

namespace phonometrica {

ScriptView::ScriptView(Runtime &rt, const Handle<Script> &script, wxWindow *parent) :
	View(parent), m_script(script), runtime(rt)
{
	SetupUi();

	if (script->has_path())
	{
		if (!filesystem::is_file(script->path()))
		{
			wxString msg = _("This file doesn't exist: ");
			msg.Append(script->path());
			wxMessageBox(msg, _("Invalid script"), wxICON_ERROR);
			return;
		}

		auto content = File::read_all(script->path());
		stc->SetText(content);
	}
	stc->notify_modification.connect(&ScriptView::OnModification, this);
}

void ScriptView::SetupUi()
{
	auto sizer = new wxBoxSizer(wxVERTICAL);
	m_toolbar = new ToolBar(this);

	wxBitmap save_icon(wxBITMAP_PNG_FROM_DATA(save));
	m_save_tool = m_toolbar->AddButton(save_icon, _("Save script (" CTRL_KEY "S)"));
	m_save_tool->Disable();
	m_toolbar->AddSeparator();

	wxBitmap run_icon(wxBITMAP_PNG_FROM_DATA(start));

	sizer->Add(m_toolbar, 0, wxEXPAND|wxALL, 10);
	auto run_tool = m_toolbar->AddButton(run_icon, _("Execute script or selection (" CTRL_KEY RETURN_KEY ")"));

	wxBitmap hint_icon(wxBITMAP_PNG_FROM_DATA(hint));
	m_hint_tool = m_toolbar->AddToggleButton(hint_icon, _("Activate auto-completion and call tips"));

	wxBitmap bytecode_icon(wxBITMAP_PNG_FROM_DATA(eye));
	auto bytecode_tool = m_toolbar->AddButton(bytecode_icon, _("View bytecode"));
	m_toolbar->AddSeparator();

	wxBitmap ident_icon(wxBITMAP_PNG_FROM_DATA(double_right));
	auto ident_tool = m_toolbar->AddButton(ident_icon, _("Indent line or selection"));
	wxBitmap unident_icon(wxBITMAP_PNG_FROM_DATA(double_left));
	auto unindent_tool = m_toolbar->AddButton(unident_icon, _("Unindent line or selection"));
	wxBitmap off_icon(wxBITMAP_PNG_FROM_DATA(toggle_off));
	auto on_tool = m_toolbar->AddButton(off_icon, _("Comment line or selection"));
	wxBitmap on_icon(wxBITMAP_PNG_FROM_DATA(toggle_on));
	auto off_tool = m_toolbar->AddButton(on_icon, _("Uncomment line or selection"));

	m_toolbar->AddStretchableSpace();
	auto help_tool = m_toolbar->AddHelpButton();

	m_toolbar->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent&) { this->Execute(); }, run_tool->GetId());
	m_toolbar->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent&) { this->Save(); }, m_save_tool->GetId());
	m_toolbar->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ScriptView::OnCommentSelection, this, on_tool->GetId());
	m_toolbar->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ScriptView::OnUncommentSelection, this, off_tool->GetId());
	m_toolbar->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ScriptView::OnIndentSelection, this, ident_tool->GetId());
	m_toolbar->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ScriptView::OnUnindentSelection, this, unindent_tool->GetId());
	m_toolbar->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ScriptView::OnOpenHelp, this, help_tool->GetId());
	m_toolbar->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ScriptView::OnViewBytecode, this, bytecode_tool->GetId());
	m_toolbar->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ScriptView::OnActivateHints, this, m_hint_tool->GetId());

	m_searchbar = new SearchBar(this, _("Find text"), true);
#ifdef __WXMAC__
	const int height = 40;
#else
	const int height = 30;
#endif
	m_searchbar->SetMaxSize(wxSize(-1, height));

	stc = new ScriptControl(this);
	stc->SetSyntaxHighlighting();
	stc->SetLineNumbering();
	sizer->Add(stc, 1, wxEXPAND, 0);
	sizer->Add(m_searchbar, 0, wxEXPAND);
	SetSizer(sizer);
    stc->SetSTCFocus(true);
    m_searchbar->Hide();

    m_searchbar->find.connect(&ScriptView::OnFind, this);
    m_searchbar->replace.connect(&ScriptView::OnReplace, this);
    m_searchbar->replace_all.connect(&ScriptView::OnReplaceAll, this);

    bool autohints = Settings::get_boolean("autohints");
    m_hint_tool->Check(autohints);
    stc->ActivateHints(autohints);
}

void ScriptView::Save()
{
	if (!m_script->has_path())
	{
		FileDialog dlg(this, _("Save script as..."), "untitled.phon", "Phonometrica script (*.phon)|*.phon", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		if (dlg.ShowModal() != wxID_OK) {
			return;
		}
		m_script->set_path(dlg.GetPath(), false);
		Project::get()->modify();
	}
	m_script->set_content(stc->GetText(), true);
	m_script->save();
	m_save_tool->Disable();
	UpdateTitle();
}

void ScriptView::OnModification()
{
	if (!m_script->modified())
	{
		m_script->set_pending_modifications();
		UpdateTitle();
		m_save_tool->Enable();
	}
}

void ScriptView::Execute()
{
	request_console();
	String code = stc->HasSelection() ? stc->GetSelectedText() : stc->GetValue();
	auto console = runtime.console;
	console->AppendNewLine();

	try
	{
		runtime.do_string(code);
	}
	catch (RuntimeError &e)
	{
		stc->ShowError(e.line_no());
		auto msg = utils::format("Error at line %\n", e.line_no());
		console->ShowErrorMessage(msg);
		console->ShowErrorMessage(e.what());
	}
	catch (std::exception &e)
	{
		console->ShowErrorMessage(e.what());
	}
	console->AddPrompt();
	console->ShowLastPosition();
}

void ScriptView::OnCommentSelection(wxCommandEvent &)
{
	AddStartCharacter("# ");
}

void ScriptView::OnUncommentSelection(wxCommandEvent &)
{
	RemoveStartCharacter("# ");
}

void ScriptView::OnIndentSelection(wxCommandEvent &)
{
	AddStartCharacter("\t");
}

void ScriptView::OnUnindentSelection(wxCommandEvent &)
{
	RemoveStartCharacter("\t");
}

void ScriptView::AddStartCharacter(const wxString &s)
{
	auto sel = stc->GetSelectedLines();
	if (sel.first < 0) return;

	for (int ln = sel.first; ln <= sel.second; ln++)
	{
		int pos = stc->PositionFromLine(ln);
		stc->InsertText(pos, s);
	}
}

void ScriptView::RemoveStartCharacter(const wxString &s)
{
	auto sel = stc->GetSelectedLines();
	if (sel.first < 0) return;

	for (int ln = sel.first; ln <= sel.second; ln++)
	{
		auto line = stc->GetLine(ln);
		if (line.StartsWith(s))
		{
			int pos = stc->PositionFromLine(ln);
			stc->Replace(pos, pos + (long)s.size(), wxEmptyString);
		}
	}
}

void ScriptView::OnOpenHelp(wxCommandEvent &)
{
	auto url = Settings::get_documentation_page("scripting");
	wxLaunchDefaultBrowser(url, wxBROWSER_NOBUSYCURSOR);
}

void ScriptView::OnViewBytecode(wxCommandEvent &)
{
	auto old_print = runtime.print;

	try
	{
		// Redirect stdout to string buffer
		String buffer;
		runtime.print = [&buffer](const String &s) {
			buffer.append(s);
		};

		auto closure = runtime.compile_string(stc->GetValue());
		runtime.disassemble(*closure, "main");
		TextViewer viewer(this, _("Bytecode viewer"), buffer);
		viewer.SetSize(FromDIP(wxSize(700, 400)));
		viewer.ShowModal();
	}
	catch (std::exception &e)
	{
		wxMessageBox(e.what(), _("Syntax error"), wxICON_ERROR);
	}

	runtime.print = old_print;
}

void ScriptView::AdjustFontSize()
{
	// FIXME: this doesn't seem to work properly, at least on linux.
	stc->InitializeFont();
	stc->Layout();
}

String ScriptView::GetPath() const
{
	return m_script->path();
}

bool ScriptView::IsModified() const
{
	return m_script->modified();
}

void ScriptView::DiscardChanges()
{
	m_script->discard_changes();
}

wxString ScriptView::GetLabel() const
{
	return m_script->label();
}

bool ScriptView::Finalize(bool autosave)
{
	if (!m_script->anchored()) {
		m_script->detach(false);
	}
	auto result = View::Finalize(autosave);
	if (result) Project::updated();
	return result;
}

void ScriptView::Find()
{
	m_searchbar->SetSearch();
}

void ScriptView::Replace()
{
	m_searchbar->SetSearchAndReplace();
}

void ScriptView::Escape()
{
	if (stc->HasFocus())
	{
		stc->CallTipCancel();
		stc->AutoCompCancel();
	}

	m_searchbar->Hide();
	stc->SetFocus();
	Layout();
}

void ScriptView::OnFind()
{
	int pos = stc->GetCurrentPos();
	stc->ClearSelections();
	String text = stc->GetValue();
	auto result = m_searchbar->Find(text, pos);

    if (result.first == text.end())
    {
        wxMessageBox(_("Text not found!"), _("Find text"));
        return;
    }
    int start = int(result.first - text.begin());
    pos = start + int(result.second - result.first);
	stc->SetSelection(start, pos);
	stc->SetCurrentPos(pos);
	int line = stc->LineFromPosition(pos);
	if (line > stc->GetFirstVisibleLine() + stc->LinesOnScreen()) {
		stc->ScrollToLine(line);
	}
}

void ScriptView::OnReplace()
{
	int pos = stc->GetCurrentPos();
	stc->ClearSelections();
	String text = stc->GetValue();
	auto result = m_searchbar->Replace(text, pos);

    if (result.first == text.end())
    {
        wxMessageBox(_("Text not found!"), _("Replace text"));
        return;
    }
    this->SetValue(text);
    int start = int(result.first - text.begin());
    pos = start + int(result.second - result.first);
	stc->SetSelection(start, pos);
	stc->SetCurrentPos(pos);
	int line = stc->LineFromPosition(pos);
	if (line > stc->GetFirstVisibleLine() + stc->LinesOnScreen()) {
		stc->ScrollToLine(line);
	}
}

void ScriptView::OnReplaceAll()
{
	String old_text = stc->GetValue();
	String new_text = old_text;
	m_searchbar->ReplaceAll(new_text);

	if (old_text != new_text)
	{
		SetValue(new_text);
	}
	else
	{
		wxMessageBox(_("Text not found!"), _("Replace text"));
	}
}

void ScriptView::OnActivateHints(wxCommandEvent &)
{
	m_hint_tool->Toggle();
	stc->ActivateHints(m_hint_tool->IsChecked());
}

void ScriptView::Undo()
{
    stc->Undo();
}

void ScriptView::Redo()
{
    stc->Redo();
}

void ScriptView::SetValue(const wxString &text)
{
	stc->SetValue(text);
	OnModification();
}


} // namespace phonometrica