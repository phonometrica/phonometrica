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
 * Created: 24/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/sizer.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/button.h>
#include <phon/application/macros.hpp>
#include <phon/gui/text_viewer.hpp>

namespace phonometrica {

TextViewer::TextViewer(wxWindow *parent, const wxString &title, const wxString &text) :
	wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	auto sizer = new wxBoxSizer(wxVERTICAL);
	auto ctrl = new wxRichTextCtrl(this);
	ctrl->SetValue(text);
	ctrl->SetEditable(false);
	wxFont font = MONOSPACE_FONT;
#if !PHON_WINDOWS
	font.SetPointSize(12);
#endif
	ctrl->SetFont(font);
	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	hsizer->AddStretchSpacer();
	auto btn = new wxButton(this, wxID_ANY, _("Close"));
	btn->SetMaxClientSize(wxSize(80, 50));
	hsizer->Add(btn, 1, wxEXPAND, 0);
	sizer->Add(ctrl, 1, wxEXPAND|wxALL, 10);
	sizer->Add(hsizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);
	sizer->AddSpacer(10);
	SetSizer(sizer);
	btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent &) { EndModal(wxID_OK); });
}
} // namespace phonometrica
