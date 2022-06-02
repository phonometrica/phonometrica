/***********************************************************************************************************************
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * purpose: main application.                                                                                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_APPLICATION_HPP
#define PHONOMETRICA_APPLICATION_HPP

#include <wx/app.h>
#include <phon/gui/main_window.hpp>

namespace phonometrica {

class Application final : public wxApp
{
public:

	explicit Application(Runtime &rt);

	bool OnInit() override;

    int OnExit() override;

    bool OnExceptionInMainLoop() override;

#ifdef __WXMAC__
	void MacOpenFile(const wxString &fileName) override;
#endif

private:

	Runtime &runtime;

	MainWindow *window;
};


} // namespace phonometrica


DECLARE_APP(phonometrica::Application);

#endif // PHONOMETRICA_APPLICATION_HPP
