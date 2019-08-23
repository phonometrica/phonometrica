/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 20/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: cross-platform printing routines.                                                                         *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_PRINT_HPP
#define PHONOMETRICA_PRINT_HPP

#include <sstream>
#include <optional>

namespace phonometrica {

class String;

namespace utils {
namespace detail {

inline
void format(std::ostringstream &msg, const char *fmt)
{
	msg << fmt;
}

template<typename T, typename... Args>
void format(std::ostringstream &msg, const char *fmt, const T &value, Args... args)
{
	for (; *fmt != '\0'; fmt++)
	{
		if (*fmt == '\\' && *(fmt + 1) == '%')
		{
			msg << "%";
			format(msg, fmt + 2, value, args...);
			return;
		}
		if (*fmt == '%')
		{
			msg << value;
			format(msg, fmt + 1, args...);
			return;
		}
		msg << *fmt;
	}
}

} // detail


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Cross platform printing function. On Windows, the string is converted to UTF-16 and printed using the OS's native
// function.
void print(FILE *file, std::string_view s);

// Replace each occurrence of '%' in a format string with a formatting argument. To print "%", use "\\%"
// Adapted from http://en.cppreference.com/w/cpp/language/parameter_pack
template<typename T, typename... Args>
std::string format(const char *fmt, const T &value, Args... args)
{
	std::ostringstream stream;
	detail::format(stream, fmt, value, args...);

	return stream.str();
}


// Similar to std::printf, but uses format() to format arguments and Detail::print() to print them.
template<typename T, typename... Args>
void printf(const char *fmt, T value, Args... args)
{
	print(stdout, format(fmt, value, args...));
};

// Similar to std::fprintf, but uses format() to format arguments and Detail::print() to print them.
template<typename T, typename... Args>
void fprintf(FILE *file, const char *fmt, T value, Args... args)
{
	print(file, format(fmt, value, args...));
};

void printf(const String &s);

std::optional<String> read_line();


}} // phonometrica::utils

#endif // PHONOMETRICA_PRINT_HPP
