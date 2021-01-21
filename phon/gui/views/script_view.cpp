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
 * Created: 14/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/artprov.h>
#include <phon/gui/views/script_view.hpp>
#include <phon/gui/macros.hpp>
#include <phon/gui/console.hpp>
#include <phon/application/settings.hpp>
#include <phon/file.hpp>

namespace phonometrica {

ScriptView::ScriptView(Runtime &rt, wxWindow *parent) :
	View(parent), runtime(rt)
{
	SetupUi();
}

ScriptView::ScriptView(Runtime &rt, const String &path, wxWindow *parent) :
	ScriptView(rt, parent)
{
	m_path = path;
	auto content = File::read_all(path);
	m_ctrl->SetText(content);
}

void ScriptView::SetupUi()
{
	auto sizer = new wxBoxSizer(wxVERTICAL);
	auto toolbar = new wxToolBar(this, wxID_ANY);
	toolbar->SetToolBitmapSize(wxSize(24, 24));

	wxBitmap save_icon(Settings::get_icon_path("save.png"), wxBITMAP_TYPE_PNG);
	m_save_tool = toolbar->AddTool(wxID_SAVE, _("Save script\tctrl+s"), save_icon, _("Save script (" CTRL_KEY "S)"));
	m_save_tool->Enable(false);
	toolbar->AddSeparator();
	wxBitmap run_icon(Settings::get_icon_path("start.png"), wxBITMAP_TYPE_PNG);
	auto run_tool = toolbar->AddTool(-1, _("Run script\tctrl+r"), run_icon, _("Run script or selection (" CTRL_KEY "R)"));
	sizer->Add(toolbar, 0, wxEXPAND | wxALL, 0);
	toolbar->AddSeparator();
	wxBitmap on_icon(Settings::get_icon_path("toggle_on.png"), wxBITMAP_TYPE_PNG);
	auto on_tool = toolbar->AddTool(-1, _("Comment selection"), on_icon, _("Comment line or selection"));
	wxBitmap off_icon(Settings::get_icon_path("toggle_off.png"), wxBITMAP_TYPE_PNG);
	auto off_tool = toolbar->AddTool(-1, _("Uncomment selection"), off_icon, _("Uncomment line or selection"));

	toolbar->Bind(wxEVT_COMMAND_TOOL_CLICKED, [this](wxCommandEvent&) { this->Run(); }, run_tool->GetId());
	toolbar->Bind(wxEVT_COMMAND_TOOL_CLICKED, [this](wxCommandEvent&) { this->Save(); }, m_save_tool->GetId());
	toolbar->Bind(wxEVT_COMMAND_TOOL_CLICKED, &ScriptView::OnCommentSelection, this, on_tool->GetId());
	toolbar->Bind(wxEVT_COMMAND_TOOL_CLICKED, &ScriptView::OnUncommentSelection, this, off_tool->GetId());

	m_ctrl = new ScriptControl(this);
	m_ctrl->SetSyntaxHighlighting();
	m_ctrl->SetLineNumbering();
	sizer->Add(m_ctrl, 1, wxEXPAND, 0);
	SetSizer(sizer);
	toolbar->Realize();
    m_ctrl->SetSTCFocus(true);
    m_ctrl->Bind(wxEVT_STC_MODIFIED, &ScriptView::OnModification, this);
}

void ScriptView::Save()
{
	if (m_path.empty()) return;
	String content = m_ctrl->GetText();
	File file(m_path, "w");
	file.write(content);
	m_save_tool->Enable(false);
	MakeTitleUnmodified();
}

bool ScriptView::Finalize()
{
	return true;
}

void ScriptView::OnModification(wxStyledTextEvent &)
{
	//MakeTitleModified();
	m_save_tool->Enable(true);
}

void ScriptView::Run()
{
	String code = m_ctrl->HasSelection() ? m_ctrl->GetSelectedText() : m_ctrl->GetValue();
	auto console = runtime.console;
	console->AppendNewLine();

	try
	{
		runtime.do_string(code);
	}
	catch (RuntimeError &e)
	{
		auto msg = utils::format("Error at line %\n", e.line_no());
		console->ShowErrorMessage(msg);
		console->ShowErrorMessage(e.what());
	}
	catch (std::exception &e)
	{
		console->ShowErrorMessage(e.what());
	}
	console->AddPrompt();
}

void ScriptView::OnCommentSelection(wxCommandEvent &)
{
	auto sel = m_ctrl->GetSelectedLines();
	if (sel.first < 0) return;
	wxString comment("#");

	for (int ln = sel.first; ln <= sel.second; ln++)
	{
		int pos = m_ctrl->PositionFromLine(ln);
		m_ctrl->InsertText(pos, comment);
	}
}

void ScriptView::OnUncommentSelection(wxCommandEvent &)
{
	auto sel = m_ctrl->GetSelectedLines();
	if (sel.first < 0) return;

	for (int ln = sel.first; ln <= sel.second; ln++)
	{
		auto line = m_ctrl->GetLine(ln);
		if (line.StartsWith("#"))
		{
			int pos = m_ctrl->PositionFromLine(ln);
			m_ctrl->Replace(pos, pos+1, wxEmptyString);
		}
	}
}


} // namespace phonometrica