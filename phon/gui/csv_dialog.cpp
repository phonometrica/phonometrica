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
 * Created: 28/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <phon/gui/csv_dialog.hpp>

namespace phonometrica {

CsvDialog::CsvDialog(wxWindow *parent, const wxString &title, bool read) :
	wxDialog(parent, wxID_ANY, title)
{
	auto sizer = new wxBoxSizer(wxVERTICAL);

	int style = read ? wxFLP_DEFAULT_STYLE : wxFLP_SAVE|wxFLP_OVERWRITE_PROMPT;
	sizer->Add(new wxStaticText(this, wxID_ANY, _("Choose file:")), 0, wxEXPAND|wxALL, 10);
	file_picker = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, "File:", "CSV (*.csv)|*.csv", wxDefaultPosition, wxDefaultSize, style);
	sizer->Add(file_picker, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

	wxArrayString choices;
	choices.Add("Semicolon");
	choices.Add("Comma");
	choices.Add("Tab");
	sep_box = new wxRadioBox(this, wxID_ANY, _("Separator"), wxDefaultPosition, wxDefaultSize, choices, 1, wxHORIZONTAL);
	sizer->Add(sep_box, 0, wxEXPAND|wxALL, 10);

	// Buttons
	auto cancel_btn = new wxButton(this, wxID_CANCEL, _("Cancel"));
	auto ok_btn = new wxButton(this, wxID_OK, _("OK"));
	wxSize btn_size(80, 50);
	ok_btn->SetMaxSize(btn_size);
	cancel_btn->SetMaxSize(btn_size);
	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	hsizer->AddStretchSpacer(1);
	hsizer->Add(cancel_btn, 1, 0, 0);
	hsizer->AddSpacer(10);
	hsizer->Add(ok_btn, 1, 0, 0);
#if PHON_MACOS
	int spacing = 15;
#else
	int spacing = 10;
#endif
	sizer->Add(hsizer, 0, wxEXPAND|wxLEFT|wxRIGHT, spacing);
	sizer->AddSpacer(10);

	cancel_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &CsvDialog::OnCancel, this);
	ok_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &CsvDialog::OnOk, this);

	SetSizer(sizer);
}

String CsvDialog::GetPath() const
{
	String path = file_picker->GetPath();

	if (!path.ends_with(".csv")) {
		path.append(".csv");
	}

	return path;
}

String CsvDialog::GetSeparator() const
{
	switch (sep_box->GetSelection())
	{
		case 0:
			return ",";
		case 1:
			return ";";
		default:
			return "\t";
	}
}

void CsvDialog::OnOk(wxCommandEvent &)
{
	EndModal(wxID_OK);
}

void CsvDialog::OnCancel(wxCommandEvent &)
{
	EndModal(wxID_CANCEL);
}
} // namespace phonometrica
