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
 * Purpose: the scanner performs lexical analysis of a chunk of source code, read from a file or from a string. The    *
 * source is expected (and assumed) to be encoded in UTF-8, and is scanned one code point at a time.                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SCANNER_HPP
#define PHONOMETRICA_SCANNER_HPP

#include <phon/runtime/compiler/token.hpp>
#include <phon/runtime/compiler/source_code.hpp>

namespace phonometrica {

class Scanner final
{
public:

    Scanner();

    // Set source code from a file on disk
    void load_file(const String &path);

    // Set source code from a string
    void load_string(const String &code);

    Token read_token();


    void report_error(const std::string &hint, intptr_t offset = 0, const char *error_type = "Syntax");

    intptr_t line_no() const { return m_line_no; }

    bool has_content() const { return !m_source->empty(); }

    std::shared_ptr<SourceCode> source_code() const { return m_source; }

private:

	// Source code (from a file or string).
    std::shared_ptr<SourceCode> m_source;

    // Current word (accumulates code points)
    String m_spelling;

    // Current line
    String m_line;

    // Current line number
    intptr_t m_line_no;

    // Current position in the current line
    String::const_iterator m_pos;

    // Current code point
    char32_t m_char;

    void reset();

    void rewind();

    void read_char();

    void get_char();

    void set_line(intptr_t index);

    void read_line();

    void skip();

    void skip_white();

    void accept();

    void scan_digits();

    void scan_string(char32_t end);

	// Same as isspace(), but does not consider '\n' as a space since it's used by the parser.
	static bool check_space(char32_t c);
};

} // namespace phonometrica

#endif // PHONOMETRICA_SCANNER_HPP
