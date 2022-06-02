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
#include <phon/runtime.hpp>
#include <phon/application/script.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Script::Script(Directory *parent, String path) :
		Document(meta::get_class<Script>(), parent, std::move(path))
{

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

const String &Script::content() const
{
	return m_content;
}

void Script::set_content(String value, bool mutate)
{
	m_content = std::move(value);
	m_content_modified |= mutate;
}

String Script::label() const
{
	using namespace filesystem;
	return m_path.empty() ? "Untitled script" :  split_ext(base_name(m_path)).first;
}

void Script::initialize(Runtime &rt)
{

}

} // namespace phonometrica
