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
#include <phon/utils/signal.hpp>

namespace phonometrica {

class SearchBar : public wxPanel
{
public:

	SearchBar(wxWindow *parent, const wxString &description, bool replace);

	bool UsesRegex() const;

	bool IsCaseSensitive() const;

	wxString GetSearchText() const;

	bool HasReplacementText() const;

	wxString GetReplacementText() const;

	void SetSearch();

	void SetSearchAndReplace();

	Signal<> execute;

protected:

	void FocusSearch();

	void OnClickCloseButton(wxMouseEvent &);

	void EnableReplace(bool value);

	wxSearchCtrl *search_ctrl;

	wxTextCtrl *repl_ctrl;

	wxButton *replace_btn, *replace_all_btn;

	wxMenuItem *case_entry = nullptr, *regex_entry = nullptr;
};

} // namespace phonometrica



#endif // PHONOMETRICA_SEARCH_BAR_HPP
