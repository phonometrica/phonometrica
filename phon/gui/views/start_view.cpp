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
 * Created: 14/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/stattext.h>
#include <phon/gui/views/start_view.hpp>
#include <phon/gui/main_window.hpp>
#include <phon/include/icons.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

StartView::StartView(wxWindow *parent, MainWindow *win) : View(parent)
{
	SetupUi(win);
}

void StartView::SetupUi(MainWindow *win)
{
#ifdef __WXGTK__
	int vspace = 40;
	auto sizer = new wxFlexGridSizer(5, 2, 0, 0);

	auto add_btn  = MakeButton(wxBITMAP_PNG_FROM_DATA(new_folder_large));
	auto open_btn = MakeButton(wxBITMAP_PNG_FROM_DATA(folder_large));
	auto doc_btn  = MakeButton(wxBITMAP_PNG_FROM_DATA(help_large));
	auto pref_btn = MakeButton(wxBITMAP_PNG_FROM_DATA(settings_large));

	int padding = 5;

	sizer->Add(add_btn,  0, wxEXPAND|wxALL, padding);
	sizer->Add(open_btn, 0, wxEXPAND|wxALL, padding);
	sizer->Add(MakeLabel("Add files to project"), 0, wxEXPAND|wxALL, padding);
	sizer->Add(MakeLabel("Open existing project"), 0, wxEXPAND|wxALL, padding);
	sizer->AddSpacer(vspace);
	sizer->AddSpacer(vspace);
	sizer->Add(doc_btn,  0, wxEXPAND|wxALL, padding);
	sizer->Add(pref_btn, 0, wxEXPAND|wxALL, padding);
	sizer->Add(MakeLabel("Documentation"), 0, wxEXPAND|wxALL, padding);
	sizer->Add(MakeLabel("Settings"), 0, wxEXPAND|wxALL, padding);

	auto main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->AddSpacer(vspace);
	main_sizer->Add(sizer, 1, wxALIGN_CENTER|wxALL);

#else
	auto sizer = new wxFlexGridSizer(2, 2, 0, 0);

	auto add_btn  = MakeButton(wxBITMAP_PNG_FROM_DATA(new_folder_large), _("Add files to project"));
	auto open_btn = MakeButton(wxBITMAP_PNG_FROM_DATA(folder_large), _("Open existing project"));
	auto doc_btn  = MakeButton(wxBITMAP_PNG_FROM_DATA(help_large), _("Read documentation"));
	auto pref_btn = MakeButton(wxBITMAP_PNG_FROM_DATA(settings_large), _("Edit preferences"));

	int padding = 25;

	sizer->Add(add_btn,  1, wxEXPAND|wxALL, padding);
	sizer->Add(open_btn, 1, wxEXPAND|wxALL, padding);
	sizer->Add(doc_btn,  1, wxEXPAND|wxALL, padding);
	sizer->Add(pref_btn, 1, wxEXPAND|wxALL, padding);

	auto main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->AddSpacer(50);
	main_sizer->Add(sizer, 1, wxALIGN_CENTER);
#endif

	SetSizer(main_sizer);

	add_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnAddFilesToProject, win);
	open_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnOpenProject, win);
	doc_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnOpenDocumentation, win);
	pref_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnEditPreferences, win);
}

#if __WXGTK__
wxButton *StartView::MakeButton(const wxBitmap &img)
{
	auto btn = new wxButton(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	btn->SetBitmap(img);

	return btn;
}

wxBoxSizer *StartView::MakeLabel(const char *label)
{
	auto sizer = new wxBoxSizer(wxHORIZONTAL);
	auto txt = new wxStaticText(this, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
//	auto font = txt->GetFont();
//	font.MakeLarger();
//	txt->SetFont(font);
	sizer->AddStretchSpacer(1);
	sizer->Add(txt, 2);
	sizer->AddStretchSpacer(1);

	return sizer;
}

#else // __WXGTK__

wxButton *StartView::MakeButton(const wxBitmap &img, const wxString &description)
{
	auto btn = new wxButton(this, wxID_ANY, description, wxDefaultPosition, wxSize(350, 150), wxNO_BORDER);
	btn->SetBitmap(img, wxLEFT);
	auto font = btn->GetFont();
	font.MakeLarger();
	btn->SetFont(font);

	return btn;
}
#endif // #if __WXGTK__

} // namespace phonometrica
