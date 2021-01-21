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
 * Created: 13/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: Control for Phonometrica's scripting language, based on Scintilla.                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SCRIPT_CONTROL_HPP
#define PHONOMETRICA_SCRIPT_CONTROL_HPP

#include <wx/stc/stc.h>

namespace phonometrica {

class ScriptControl final : public wxStyledTextCtrl
{
public:

	explicit ScriptControl(wxWindow *parent);

	void SetLineNumbering();

	void SetSyntaxHighlighting();

	void AddTypeName(const wxString &kw);

	void HideMargin();

	std::pair<int,int> GetSelectedLines() const;

private:

	void InitFont();
	void OnChange(wxStyledTextEvent &);
};


} // namespace phonometrica

#endif // PHONOMETRICA_SCRIPT_CONTROL_HPP
