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
 * Created: 19/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/button.h>
#include <phon/gui/pref/preferences_dialog.hpp>

namespace phonometrica {

PreferencesDialog::PreferencesDialog(wxWindow *parent, const wxString &title) :
	wxDialog(parent, wxID_ANY, title)
{
	m_sizer = new wxBoxSizer(wxVERTICAL);
	m_book = new wxNotebook(this, wxID_ANY);
	m_sizer->Add(m_book, 1, wxEXPAND|wxALL, 10);
	CreateButtons();
	SetSizer(m_sizer);
}

void PreferencesDialog::AddPage(wxWindow *page, const wxString &title)
{
	m_book->AddPage(page, title);
}

void PreferencesDialog::CreateButtons()
{
	auto reset_btn = new wxButton(this, wxID_RESET, _("Reset"));
	auto cancel_btn = new wxButton(this, wxID_CANCEL, _("Cancel"));
	auto ok_btn = new wxButton(this, wxID_OK, _("OK"));
	wxSize btn_size(80, 50);
	ok_btn->SetMaxSize(btn_size);
	cancel_btn->SetMaxSize(btn_size);
	reset_btn->SetMaxSize(btn_size);
	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	hsizer->Add(reset_btn, 1, 0, 0);
	hsizer->AddStretchSpacer(1);
	hsizer->Add(cancel_btn, 1, 0, 0);
	hsizer->AddSpacer(10);
	hsizer->Add(ok_btn, 1, 0, 0);
	m_sizer->Add(hsizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 20);
	m_sizer->AddSpacer(10);

	reset_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PreferencesDialog::OnReset, this);
	cancel_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PreferencesDialog::OnCancel, this);
	ok_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PreferencesDialog::OnOk, this);
}

void PreferencesDialog::OnCancel(wxCommandEvent &)
{
	DoCancel();
	EndModal(wxID_CANCEL);
}

void PreferencesDialog::OnReset(wxCommandEvent &)
{
	DoReset();
	EndModal(wxID_RESET);
}

void PreferencesDialog::OnOk(wxCommandEvent &)
{
	DoOk();
	EndModal(wxID_OK);
}

} // namespace phonometrica