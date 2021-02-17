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
#include <phon/gui/sizer.hpp>
#include <phon/gui/search_bar.hpp>
#include <phon/include/icons.hpp>

namespace phonometrica {

SearchBar::SearchBar(wxWindow *parent, const wxString &description, bool replace) :
	wxPanel(parent, wxID_ANY)
{
	const int spacing = 1;
#ifdef __WXMAC__
	const int height = 40;
#else
	const int height = 30;
#endif
	auto sizer = new HBoxSizer;

	search_ctrl = new wxSearchCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	search_ctrl->SetDescriptiveText(description);
	search_ctrl->SetSize(FromDIP(wxSize(-1, height)));
	search_ctrl->ShowCancelButton(true);
	auto menu = new wxMenu;
	auto case_id = wxNewId();
	case_entry = menu->AppendCheckItem(case_id, _("Case-sensitive"));
//	auto regex_id = wxNewId();
//	regex_entry = menu->AppendCheckItem(regex_id, _("Use regular expressions"));
	search_ctrl->SetMenu(menu);

	repl_ctrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	repl_ctrl->Enable(replace);
	repl_ctrl->SetSize(FromDIP(wxSize(-1, height)));
	auto find_btn = new wxButton(this, wxID_ANY, _("Find"));
	replace_btn = new wxButton(this, wxID_ANY, _("Replace"));
	replace_all_btn = new wxButton(this, wxID_ANY, _("Replace all"));
	auto icon = new wxStaticBitmap(this, wxID_ANY, wxBITMAP_PNG_FROM_DATA(close));

	sizer->AddSpacer(5);
	sizer->Add(search_ctrl, 1, 0, 0);
	sizer->Add(find_btn, 0, wxLEFT|wxALIGN_CENTER, spacing);
	sizer->Add(repl_ctrl, 1, wxLEFT, spacing);
	sizer->Add(replace_btn, 0, wxLEFT | wxALIGN_CENTER, spacing);
	sizer->Add(replace_all_btn, 0, wxLEFT | wxALIGN_CENTER | wxRIGHT, spacing);
	sizer->Add(icon, 0, wxALIGN_CENTER);
	SetSizer(sizer);

	icon->Bind(wxEVT_LEFT_UP, &SearchBar::OnClickCloseButton, this);
	find_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent &) { execute(); });
	search_ctrl->Bind(wxEVT_SEARCH, [this](wxCommandEvent &) { execute(); });
}

bool SearchBar::UsesRegex() const
{
	return regex_entry != nullptr && regex_entry->IsChecked();
}

bool SearchBar::IsCaseSensitive() const
{
	return case_entry->IsChecked();
}

bool SearchBar::HasReplacementText() const
{
	return repl_ctrl->IsEnabled();
}

wxString SearchBar::GetSearchText() const
{
	return search_ctrl->GetValue();
}

wxString SearchBar::GetReplacementText() const
{
	return repl_ctrl->GetValue();
}

void SearchBar::FocusSearch()
{
	search_ctrl->SetFocus();
}

void SearchBar::SetSearch()
{
	EnableReplace(false);
	Show();
	GetParent()->Layout();
	FocusSearch();
}

void SearchBar::SetSearchAndReplace()
{
	EnableReplace(true);
	Show();
	GetParent()->Layout();
	FocusSearch();
}

void SearchBar::OnClickCloseButton(wxMouseEvent &)
{
	Hide();
	GetParent()->Layout();
}

void SearchBar::EnableReplace(bool value)
{
	repl_ctrl->Enable(value);
	replace_btn->Enable(value);
	replace_all_btn->Enable(value);
}

} // namespace phonometrica
