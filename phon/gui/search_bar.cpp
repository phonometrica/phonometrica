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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <phon/gui/sizer.hpp>
#include <phon/gui/search_bar.hpp>
#include <phon/include/icons.hpp>

namespace phonometrica {

SearchBar::SearchBar(wxWindow *parent, const wxString &description, bool replace) :
	wxPanel(parent, wxID_ANY)
{
	const int spacing = 1;
	const int height = 30;
	auto sizer = new HBoxSizer;

	search_ctrl = new wxSearchCtrl(this, wxID_ANY);
	search_ctrl->SetDescriptiveText(description);
	search_ctrl->SetSize(FromDIP(wxSize(-1, height)));
	search_ctrl->ShowCancelButton(true);
	auto menu = new wxMenu;
	auto case_id = wxNewId();
	case_entry = menu->AppendCheckItem(case_id, _("Case-sensitive"));
	auto regex_id = wxNewId();
	regex_entry = menu->AppendCheckItem(regex_id, _("Use regular expressions"));
	search_ctrl->SetMenu(menu);

	repl_checkbox = new wxCheckBox(this, wxID_ANY, _("Replace:"));
	repl_checkbox->SetValue(replace);
	repl_checkbox->SetSize(FromDIP(wxSize(-1, height)));
	repl_ctrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	repl_ctrl->Enable(replace);
	repl_ctrl->SetSize(FromDIP(wxSize(-1, height)));
	auto find_btn = new wxButton(this, wxID_ANY, _("Find"));
	auto repl_btn = new wxButton(this, wxID_ANY, _("Replace"));
	auto repl_all_btn = new wxButton(this, wxID_ANY, _("Replace all"));
	auto icn = new wxStaticBitmap(this, wxID_ANY, wxBITMAP_PNG_FROM_DATA(close));

	sizer->AddSpacer(5);
	sizer->Add(new wxStaticText(this, wxID_ANY, _("Find:")), 0, wxLEFT|wxALIGN_CENTER, spacing);
	sizer->Add(search_ctrl, 1, wxLEFT, spacing+5);
	sizer->Add(repl_checkbox, 0, wxLEFT|wxALIGN_CENTER, spacing+5);
	sizer->Add(repl_ctrl, 1, wxLEFT, spacing);
	sizer->Add(find_btn, 0, wxLEFT, spacing);
	sizer->Add(repl_btn, 0, wxLEFT, spacing);
	sizer->Add(repl_all_btn, 0, wxLEFT|wxRIGHT, spacing);
	sizer->Add(icn, 0, wxALIGN_CENTER);
	SetSizer(sizer);

	repl_checkbox->Bind(wxEVT_CHECKBOX, [=](wxCommandEvent &e) { repl_ctrl->Enable(repl_checkbox->GetValue()); });
}

bool SearchBar::UsesRegex() const
{
	return regex_entry->IsChecked();
}

bool SearchBar::IsCaseSensitive() const
{
	return case_entry->IsChecked();
}

bool SearchBar::HasReplace() const
{
	return repl_checkbox->GetValue();
}

String SearchBar::GetSearchText() const
{
	return search_ctrl->GetValue();
}

String SearchBar::GetReplacementText() const
{
	return repl_ctrl->GetValue();
}

void SearchBar::FocusSearch()
{
	search_ctrl->SetFocus();
}

void SearchBar::SetSearch()
{
	repl_checkbox->SetValue(false);
	repl_ctrl->Enable(false);
	Show();
	FocusSearch();
}

void SearchBar::SetSearchAndReplace()
{
	repl_checkbox->SetValue(true);
	repl_ctrl->Enable(true);
	Show();
	FocusSearch();
}
} // namespace phonometrica
