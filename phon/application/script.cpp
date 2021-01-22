/***********************************************************************************************************************
 *                                                                                                                     *
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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/file.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/application/script.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Script::Script(VFolder *parent, String path) :
		VFile(parent, std::move(path))
{

}

bool Script::is_script() const
{
	return true;
}

const char *Script::class_name() const
{
	return "Script";
}

void Script::load()
{
	m_content = File::read_all(m_path);
}

void Script::write()
{
	File file(m_path, File::Write, Encoding::Utf8);
	file.write(m_content);
	file.close();
}

void Script::from_xml(xml_node root, const String &project_dir)
{
	static const std::string_view path_tag("Path");

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == path_tag)
		{
			String path(node.text().get());
			Project::interpolate(path, project_dir);
			m_path = std::move(path);
		}
	}
}

const String &Script::content() const
{
	return m_content;
}

void Script::set_content(String value, bool mutate)
{
	m_content = std::move(value);
	m_content_modified |= mutate;
}

} // namespace phonometrica
