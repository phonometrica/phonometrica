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
 * Created: 13/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/application.hpp>

namespace phonometrica {

Application::Application(Runtime &rt) :
	wxApp(), runtime(rt)
{

}

bool Application::OnInit()
{
	wxImage::AddHandler(new wxPNGHandler());

	try
	{
		window = new MainWindow(runtime, "Phonometrica");
		SetTopWindow(window);
		window->Layout();
		window->Show();
		window->PostInitialize();
		// Bind OnResize after the window is properly sized
		Bind(wxEVT_SIZE, &MainWindow::OnResize, window);
	}
	catch (std::exception &e)
	{
		wxMessageBox(wxString(e.what()), _("Initialization failed"), wxICON_ERROR);
		return false;
	}

	return true;
}

int Application::OnExit()
{
	return 0;
}

#ifdef __WXMAC__
void Application::MacOpenFile(const wxString &fileName)
{
	// TODO: implement dropping files in project manager on macos
	wxApp::MacOpenFile(fileName);
}
#endif

} // namespace phonometrica