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
#include <phon/gui/macros.hpp>
#include <phon/gui/console.hpp>
#include <phon/file.hpp>

namespace phonometrica {

static size_t QUEUE_LIMIT = 50;

Console::Console(Runtime &rt, wxWindow *parent) :
	wxPanel(parent, wxNewId(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE), runtime(rt), prompt(">> ")
{
	SetIO();
	SetSizer(m_sizer);
	AddPrompt();

	// Redirect stdout to console.
	if (!rt.is_text_mode())
	{
		rt.print = [&](const String &s) {
			this->Append(s);
			this->text_written = true;
		};

		// Clear console.
		rt.add_global("reset", [=](Runtime &, std::span<Variant>) { this->Clear(); return Variant(); }, {});
	}
	rt.console = this;
}

void Console::SetIO()
{
	m_ctrl = new wxRichTextCtrl(this);
	m_sizer = new wxBoxSizer(wxVERTICAL);
	m_sizer->Add(m_ctrl, 1, wxEXPAND, 0);
	wxFont font = MONOSPACE_FONT;
	font.SetPointSize(12);
	m_ctrl->SetFont(font);
	m_ctrl->Bind(wxEVT_KEY_DOWN, &Console::OnKeyDown, this);
}

void Console::OnKeyDown(wxKeyEvent &e)
{
	// Don't write in the prompt (">> ")
	if (m_ctrl->GetInsertionPoint() < (long)prompt.size()) return;

	int key = e.GetKeyCode();

	if (key == WXK_UP) {

		if (history_pos > 0)
		{
			--history_pos;

			if (history_pos < history.size())
			{
				ResetLastLine(history.at(history_pos));
			}
		}

		return;
	}
	else if (key == WXK_DOWN)
	{
		if (! history.empty())
		{
			if (history_pos < history.size()) { ++history_pos; }

			if (history_pos < history.size())
			{
				const wxString &line = history.at(history_pos);
				ResetLastLine(line);
			}
			else
			{
				// Show an empty line if at the end of the queue
				ResetLastLine(wxString());
			}
		}

		return;
	}
	else if (key == WXK_LEFT || key == WXK_BACK)
	{
		long min_pos = m_ctrl->GetLastPosition() - m_ctrl->GetLineText(m_ctrl->GetNumberOfLines()-1).Length() + 3;

		if (m_ctrl->GetInsertionPoint() <= min_pos)
		{
			return;
		}
	}
	else if (key == WXK_RETURN)
	{
		GrabLine();
		return;
	}

	e.Skip();
}

void Console::GrabLine()
{
	text_written = false;
	wxString ln = m_ctrl->GetLineText(m_ctrl->GetNumberOfLines()-1);
	auto pos = ln.Find(prompt);

	if (pos == wxNOT_FOUND)
	{
		wxMessageBox("Invalid prompt in Console::GrabLine(): please report this to the authors", _("Internal error"), wxICON_ERROR);
		return;
	}
	ln = ln.Mid(pos + prompt.size()).Trim();

	if (ln.IsEmpty()) {
		AddPrompt(); return;
	}

	String line = ln;
	RunCode(line);

	while (history.size() > QUEUE_LIMIT) { history.pop_front(); }

	if (! line.empty())
	{
		history.push_back(line);
		history_pos = history.size(); // current position outside of history
	}

	GoToEnd();
}

void Console::ShowErrorMessage(const wxString &msg)
{
	m_ctrl->BeginTextColour(wxColour(255, 0, 0));
	m_ctrl->WriteText(msg);
	m_ctrl->EndTextColour();
}

void Console::Clear()
{
	m_ctrl->SetValue("");
}

void Console::AddPrompt()
{
	wxString ln = m_ctrl->GetLineText(m_ctrl->GetNumberOfLines()-1);
	// If an EOL character has been appended, the last line is empty, so we check that
	// in order to determine whether we need to append a new line.
	if (!ln.IsEmpty() && m_ctrl->GetLastPosition() > 0) {
		AppendNewLine();
	}
	Append(prompt);
	GoToEnd();
}

void Console::ResetLastLine(wxString text)
{
	long line_count = m_ctrl->GetNumberOfLines();
	long start = m_ctrl->XYToPosition(0, line_count-1);
	long end = m_ctrl->GetLastPosition();
	text.Prepend(prompt);
	m_ctrl->Replace(start, end, text);
	GoToEnd();
}

void Console::ChopNewLine()
{
	auto end = (long)m_ctrl->GetValue().length();
	m_ctrl->Remove(end-1, end);
}

void Console::RunScript(const String &path)
{
	auto content = File::read_all(path);
	RunCode(content);
}

void Console::RunCode(const String &code)
{
	AppendNewLine();

	try
	{
		runtime.do_string(code);
	}
	catch (std::exception &e)
	{
		ShowErrorMessage(e.what());
	}

	AddPrompt();
}

} // namespace phonometrica
