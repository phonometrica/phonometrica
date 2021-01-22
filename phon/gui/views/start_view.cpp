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
#include <phon/application/settings.hpp>
#include <phon/utils/file_system.hpp>

#ifndef __WXGTK__

namespace phonometrica {

StartView::StartView(wxWindow *parent, MainWindow *win) : View(parent)
{
	auto sizer = new wxFlexGridSizer(2, 2, 0, 0);

	auto add_btn  = MakeButton("new_folder.png", _("Add files to project"));
	auto open_btn = MakeButton("folder.png", _("Open existing project"));
	auto doc_btn  = MakeButton("help.png", _("Read documentation"));
	auto pref_btn = MakeButton("settings.png", _("Edit preferences"));

	int padding = 25;

	sizer->Add(add_btn,  1, wxEXPAND|wxALL, padding);
	sizer->Add(open_btn, 1, wxEXPAND|wxALL, padding);
	sizer->Add(doc_btn,  1, wxEXPAND|wxALL, padding);
	sizer->Add(pref_btn, 1, wxEXPAND|wxALL, padding);


	add_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnAddFilesToProject, win);
	open_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnOpenProject, win);
	doc_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnOpenDocumentation, win);
	pref_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnEditPreferences, win);

	auto main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->AddSpacer(50);
	main_sizer->Add(sizer, 1, wxALIGN_CENTER);

	SetSizer(main_sizer);
}

wxButton *StartView::MakeButton(std::string_view filename, const wxString &description)
{
	String p = filesystem::join(Settings::icon_directory(), "100x100");
	wxString path = filesystem::join(p, filename);
	wxBitmap icon(path, wxBITMAP_TYPE_PNG);
	auto btn = new wxButton(this, wxID_ANY, description, wxDefaultPosition, wxSize(300, 150), wxNO_BORDER);
	btn->SetBitmap(icon, wxLEFT);
	auto font = btn->GetFont();
	font.MakeLarger();
	btn->SetFont(font);

	return btn;
}

} // namespace phonometrica

#endif // __WXGTK__