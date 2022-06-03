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
 * Created: 18/07/2019                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/file.hpp>
#include <phon/runtime/compiler/source_code.hpp>

namespace phonometrica {

SourceCode::SourceCode()
{

}

void SourceCode::load_file(const String &path)
{
    File infile(path, File::Mode::Read);
    m_lines = infile.read_lines();
    this->m_path = path;
}

void SourceCode::load_code(const String &code)
{
    m_lines = code.split("\n");
    // The splitter is stripped when splitting a string, so we need to put it back.
    for (auto &ln : m_lines)
    {
        ln.append('\n');
    }

    m_path.clear();
}

String SourceCode::filename() const
{
    return m_path.empty() ? String("string buffer") : m_path;
}

String SourceCode::get_line(intptr_t index) const
{
    return m_lines[index];
}

intptr_t SourceCode::size() const
{
    return m_lines.size();
}

void SourceCode::report_error(const char *error_type, intptr_t line_no, const std::string &hint)
{
    assert(line_no > 0);
    String line = m_lines[line_no];
    line.rtrim();
    auto message = utils::format("[%] File \"%\" at line %\n\t%", error_type, this->filename(), line_no, line);

    if (!hint.empty())
    {
        message.append("\nHint: ");
        message.append(hint);
    }

    throw error(message);
}

} // namespace phonometrica