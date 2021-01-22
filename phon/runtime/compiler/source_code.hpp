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
 * Created: 18/07/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: represents a chunk of source code, loaded from memory or from a text file.                                 *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SOURCE_CODE_HPP
#define PHONOMETRICA_SOURCE_CODE_HPP

#include <phon/string.hpp>

namespace phonometrica {

class SourceCode final
{
public:

    SourceCode();

    String filename() const;

    void dispose() { delete this; }

    bool empty() const { return m_lines.empty(); }

    const String &path() const { return m_path; }

    // Set source code from a file on disk
    void load_file(const String &path);

    // Set source code from a string
    void load_code(const String &code);

    String get_line(intptr_t index) const;

    intptr_t size() const;

    // This is used by AST visitors. It is less detailed than an error reported by the scanner
    // since we can't use the token's position, but it's better than nothing...
    void report_error(const char *error_type, intptr_t line_no,  const std::string &hint = std::string());

private:

    // Current file (empty if memory buffer)
    String m_path;

    // Current file
    Array<String> m_lines;

};

} // namespace phonometrica

#endif // PHONOMETRICA_SOURCE_CODE_HPP
