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
#include <phon/gui/views/linux_start_view.hpp>
#include <phon/gui/main_window.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/application/settings.hpp>

#ifdef __WXGTK__

namespace phonometrica {

StartView::StartView(wxWindow *parent, MainWindow *win) : View(parent)
{
	int vspace = 40;
	auto sizer = new wxFlexGridSizer(5, 2, 0, 0);

	auto add_btn  = MakeButton("new_folder.png");
	auto open_btn = MakeButton("folder.png");
	auto doc_btn  = MakeButton("help.png");
	auto pref_btn = MakeButton("settings.png");

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

//	add_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnAddFilesToProject, win);
//	open_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnOpenProject, win);
	doc_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnOpenDocumentation, win);
//	pref_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnEditPreferences, win);

	auto main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->AddSpacer(vspace);
	main_sizer->Add(sizer, 1, wxALIGN_CENTER|wxALL);

	SetSizer(main_sizer);
}

wxButton *StartView::MakeButton(std::string_view filename)
{
	// TODO: set icon path with settings
	wxString path = filesystem::join(Settings::icon_directory(), "100x100", filename);
	wxBitmap icon(path, wxBITMAP_TYPE_PNG);
	auto btn = new wxButton(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	btn->SetBitmap(icon);

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
} // namespace phonometrica

#endif // __WXGTK__