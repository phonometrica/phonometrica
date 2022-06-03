/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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
 * Created: 15/04/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/button.h>
#include <phon/gui/channel_dialog.hpp>
#include <phon/gui/sizer.hpp>

namespace phonometrica {

ChannelDialog::ChannelDialog(wxWindow *parent, int nchannel, const std::vector<int> &visible_channels) :
	wxDialog(parent, wxID_ANY, _("Select channels..."), wxDefaultPosition, wxSize(300, -1), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	wxArrayString choices;
	choices.Add(_("Select channels"));
	choices.Add(_("Average channels"));
	selector = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
	selector->SetSelection(0);
	auto sizer = new VBoxSizer;
	sizer->Add(selector, 0, wxALL|wxEXPAND, 10);

	for (int i = 1; i <= nchannel; i++)
	{
		auto box = new wxCheckBox(this, wxID_ANY, wxString::Format(_("channel %d"), i));
		bool enable = (std::find(visible_channels.begin(), visible_channels.end(), i) != visible_channels.end());
		box->SetValue(enable);
		sizer->Add(box, 0, wxLEFT|wxRIGHT|wxBOTTOM, 10);
		channels.push_back(box);
	}

	bool mono = (nchannel == 1);
	if (mono)
	{
		selector->Enable(false);
		channels[0]->Enable(false);
	}

	auto btn_sizer = new HBoxSizer;
	btn_sizer->AddStretchSpacer();
	auto ok_btn = new wxButton(this, wxID_OK, _("OK"));
	auto cancel_btn = new wxButton(this, wxID_CANCEL, _("Cancel"));
	btn_sizer->Add(cancel_btn);
	btn_sizer->AddSpacer(5);
	btn_sizer->Add(ok_btn);
	sizer->AddStretchSpacer();
	sizer->Add(btn_sizer, 0, wxEXPAND|wxALL, 10);
	SetSizer(sizer);
	Bind(wxEVT_CHOICE, &ChannelDialog::OnChoiceChanged, this);
}

void ChannelDialog::OnChoiceChanged(wxCommandEvent &)
{
	bool clickable = (selector->GetSelection() == 0);

	for (auto box : channels) {
		box->Enable(clickable);
	}
}

std::vector<int> ChannelDialog::GetSelectedChannels() const
{
	std::vector<int> sel;

	if (selector->GetSelection() == 1) {
		return sel;
	}

	for (int i = 0; i < (int)channels.size(); i++)
	{
		if (channels[i]->GetValue()) {
			sel.push_back(i+1);
		}
	}

	return sel;
}
} // namespace phonometrica
