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
 * purpose: Control for Phonometrica's scripting language, based on Scintilla.                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SCRIPT_CONTROL_HPP
#define PHONOMETRICA_SCRIPT_CONTROL_HPP

#include <unordered_map>
#include <wx/stc/stc.h>
#include <phon/utils/signal.hpp>

namespace phonometrica {

class ScriptControl final : public wxStyledTextCtrl
{
public:

	explicit ScriptControl(wxWindow *parent);

	void SetLineNumbering();

	void SetSyntaxHighlighting();

	void AddTypeNames();

	void HideMargin();

	std::pair<int,int> GetSelectedLines() const;

	void InitializeFont();

	void ShowError(intptr_t line_no);

	void WriteNewLine();

	void ActivateHints(bool value);

	Signal<> notify_modification;

private:

	using CallTipList = std::vector<wxString>;
	using CallTipMap = std::unordered_map<wxString, CallTipList>;

	void OnCharAdded(wxStyledTextEvent &);

	void OnChange(wxStyledTextEvent &);

	wxString GetSubstring(int from, int to);

	void InitializeCallTips();

	void OnCallTipClicked(wxStyledTextEvent &e);

	void OnDoubleClick(wxStyledTextEvent &e);

	int error_indicator = 0;

	int selection_style = 0;

	bool has_hints = false;

	std::pair<int,int> error_selection;

	CallTipMap calltips;

	int calltip_position = 0;

	CallTipList *calltip_list = nullptr;

	CallTipList::iterator current_calltip;
};


} // namespace phonometrica

#endif // PHONOMETRICA_SCRIPT_CONTROL_HPP
