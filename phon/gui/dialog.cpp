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
 * Created: 29/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/dialog.hpp>
#include <phon/application/settings.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

FileDialog::FileDialog(wxWindow *parent, const wxString &message, const wxString &default_file, const wxString &wildcard, long style) :
	wxFileDialog(parent, message, Settings::get_string("last_directory"), default_file, wildcard, style)
{

}

wxString FileDialog::GetPath() const
{
	String path = wxFileDialog::GetPath();
	Settings::set_value("last_directory", filesystem::directory_name(path));

	return wxFileDialog::GetPath();
}

void FileDialog::GetPaths(wxArrayString &paths) const
{
	wxFileDialog::GetPaths(paths);

	if (!paths.IsEmpty())
	{
		String path = paths.front();
		Settings::set_value("last_directory", filesystem::directory_name(path));
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DirDialog::DirDialog(wxWindow *parent, const wxString &message, long style) :
	wxDirDialog(parent, message, Settings::get_string("last_directory"), style)
{

}

wxString DirDialog::GetPath() const
{
	Settings::set_value("last_directory", String(wxDirDialog::GetPath()));
	return wxDirDialog::GetPath();
}

void DirDialog::GetPaths(wxArrayString &paths) const
{
	wxDirDialog::GetPaths(paths);

	if (!paths.IsEmpty()) {
		Settings::set_value("last_directory", String(paths.front()));
	}
}
} // namespace phonometrica
