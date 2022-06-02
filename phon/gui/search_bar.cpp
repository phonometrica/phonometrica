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
#include <phon/regex.hpp>

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

	search_ctrl = new wxSearchCtrl(this, wxID_ANY, "");
	search_ctrl->SetDescriptiveText(description);
	search_ctrl->SetSize(FromDIP(wxSize(-1, height)));
	search_ctrl->ShowCancelButton(true);
	auto menu = new wxMenu;
	auto case_id = wxNewId();
	case_entry = menu->AppendCheckItem(case_id, _("Case-sensitive"));
	auto regex_id = wxNewId();
	regex_entry = menu->AppendCheckItem(regex_id, _("Use regular expressions"));
	search_ctrl->SetMenu(menu);

	repl_ctrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
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

	auto col = GetBackgroundColour();
	double factor = 0.85;
	auto r = col.Red();
	auto g = col.Green();
	auto b = col.Blue();
	r = (unsigned char)(r + factor * (255 - r));
	g = (unsigned char)(g + factor * (255 - g));
	b = (unsigned char)(b + factor * (255 - b));
	col = wxColor(r,g,b);

	icon->Bind(wxEVT_ENTER_WINDOW, [icon,col](wxMouseEvent &) { icon->SetBackgroundColour(col); icon->Refresh(); });
	icon->Bind(wxEVT_LEAVE_WINDOW, [icon,this](wxMouseEvent &) { icon->SetBackgroundColour(GetBackgroundColour()); icon->Refresh(); });
	icon->Bind(wxEVT_LEFT_UP, &SearchBar::OnClickCloseButton, this);


	search_ctrl->Bind(wxEVT_SEARCH, [this](wxCommandEvent &) { this->find(); });
	repl_ctrl->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent &) { this->replace(); });
	find_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent &) { this->find(); });
	replace_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent &) { this->replace(); });
	replace_all_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent &) { this->replace_all(); });
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

SearchBar::Region SearchBar::Find(const String &text, intptr_t start)
{
	String target = this->GetSearchText();
	String::const_iterator it1 = text.cbegin() + start;
	String::const_iterator it2;

	if (UsesRegex())
	{
		auto flags = IsCaseSensitive() ? Regex::None :  Regex::Caseless;
		Regex re(target, flags);

		if (re.match(text, it1)) {
			return { re.capture_start_iter(0), re.capture_end_iter(0) };
		}
		else {
			return { text.cend(), text.cend() };
		}
	}
	else if (this->IsCaseSensitive())
	{
		it1 = text.find(target, it1);
		it2 = it1 + target.size(); // invalid if it1 == text.end()
	}
	else
	{
		it1 = text.ifind(target, it1, &it2);
	}

	return { it1, it2 };
}

SearchBar::Region SearchBar::Replace(String &text, intptr_t start)
{
	// We roll our own search function because Scintilla's is not Unicode-aware (e.g. "été" and "ÉTÉ" don't match).
	auto result = Find(text, start);

	if (result.first == text.end()) {
		return result;
	}
	String replacement = GetReplacementText();
	text.replace(result.first, result.second, replacement);

	return { result.first, result.first + replacement.size() };
}

void SearchBar::ReplaceAll(String &text)
{
	String target = GetSearchText();
	String replacement = GetReplacementText();

	if (UsesRegex())
	{
		auto flags = IsCaseSensitive() ? Regex::None :  Regex::Caseless;
		Regex re(GetSearchText(), flags);
		auto it = text.cbegin();
		while (re.match(text, it))
		{
			auto offset = intptr_t(it - text.cbegin());
			auto start = re.capture_start_iter(0);
			auto end = re.capture_end_iter(0);
			offset +=  replacement.size();
			text.replace(start, end, replacement);
			it = text.cbegin() + offset;
		}
	}
	else if (IsCaseSensitive())
	{
		text.replace(target, replacement);
	}
	else
	{
		String::const_iterator it1 = text.cbegin();
		String::const_iterator it2;

		while (true)
		{
			it1 = text.ifind(target, it1, &it2);
			if (it1 != text.cend())
			{
				auto offset = intptr_t(it1 - text.cbegin());
				offset += replacement.size();
				text.replace(it1, it2, replacement);
				it1 = text.cbegin() + offset;
			}
			else
			{
				break;
			}
		}
	}
}

} // namespace phonometrica
