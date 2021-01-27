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
 * purpose: Scripting console, located at the bottom of the main window.                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CONSOLE_HPP
#define PHONOMETRICA_CONSOLE_HPP

#include <deque>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/richtext/richtextctrl.h>
#include <phon/runtime.hpp>

namespace phonometrica {

class Console final : public wxPanel
{
public:

	Console(Runtime &rt, wxWindow *parent);

	void ShowErrorMessage(const wxString &msg);

	void AddPrompt();

	void AppendNewLine() { static wxString eol("\n"); m_ctrl->WriteText(eol); }

	void ChopNewLine();

	void RunScript(const String &path);

private:

	void SetIO();

	void OnKeyDown(wxKeyEvent &e);

	void GrabLine();

	void Clear();

	void ResetLastLine(wxString text);

	void GoToEnd() { m_ctrl->SetInsertionPointEnd(); }

	void Append(const wxString &text) { m_ctrl->WriteText(text); }

	void RunCode(const String &code);

	wxRichTextCtrl *m_ctrl;

	wxSizer *m_sizer;

	size_t history_pos = 0;

	std::deque<wxString> history;

	Runtime &runtime;

	wxString prompt;

	// Check whether some text has been written to stdout.
	// This is used to add a new prompt line when running a script,
	// if some text has been printed to the control.
	bool text_written = false;
};

} // namespace phonometrica

#endif // PHONOMETRICA_CONSOLE_HPP
