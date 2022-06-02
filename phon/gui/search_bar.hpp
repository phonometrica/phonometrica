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
 * Created: 07/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Find/Replace dialog.                                                                                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SEARCH_BAR_HPP
#define PHONOMETRICA_SEARCH_BAR_HPP

#include <wx/panel.h>
#include <wx/menu.h>
#include <wx/srchctrl.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <phon/string.hpp>
#include <phon/utils/signal.hpp>

namespace phonometrica {

class SearchBar : public wxPanel
{
public:

	// Used to locate search matches
	using Region = std::pair<String::const_iterator, String::const_iterator>;

	SearchBar(wxWindow *parent, const wxString &description, bool replace);

	bool HasReplacementText() const;

	void SetSearch();

	void SetSearchAndReplace();

	Region Find(const String &text, intptr_t start);

	Region Replace(String &text, intptr_t start);

	void ReplaceAll(String &text);

	Signal<> find, replace, replace_all;

protected:

	void FocusSearch();

	void OnClickCloseButton(wxMouseEvent &);

	void EnableReplace(bool value);

	bool UsesRegex() const;

	bool IsCaseSensitive() const;

	wxString GetSearchText() const;

	wxString GetReplacementText() const;

	wxSearchCtrl *search_ctrl;

	wxTextCtrl *repl_ctrl;

	wxButton *replace_btn, *replace_all_btn;

	wxMenuItem *case_entry = nullptr, *regex_entry = nullptr;
};

} // namespace phonometrica



#endif // PHONOMETRICA_SEARCH_BAR_HPP
