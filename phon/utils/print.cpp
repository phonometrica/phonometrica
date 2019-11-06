/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 20/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/string.hpp>
#include <phon/utils/print.hpp>

namespace phonometrica { namespace utils {

void print(FILE *file, std::string_view s)
{
#if PHON_WINDOWS
	auto utf16 = String::to_wide(s);
	fwprintf(file, L"%s", utf16.data());
#else
	std::fprintf(file, "%s", s.data());
#endif
}

void printf(const String &s)
{
	print(stdout, s);
}

std::optional<String> read_line()
{
#if PHON_WINDOWS
	static wchar_t line[500], *p;

	p = fgetws(line, sizeof line, stdin);
	if (p)
	{
		auto n = std::char_traits<wchar_t>::length(line);
		if (n > 0 && line[n - 1] == '\n')
			line[--n] = 0;
		return String(line, n);

	}
#else
	static char line[500], *p;

	p = fgets(line, sizeof line, stdin);
	if (p)
	{
		auto n = std::char_traits<char>::length(line);
		if (n > 0 && line[n - 1] == '\n')
			line[--n] = 0;
		return String(line, n);
	}
#endif

	return std::optional<String>();
}


}} // namespace phonometrica::utils
