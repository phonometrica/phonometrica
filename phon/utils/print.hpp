/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                      *
 *                                                                                                                    *
 * The contents of this file are subject to the Mozilla Public License Version 2.0 (the "License"); you may not use   *
 * this file except in compliance with the License. You may obtain a copy of the License at                           *
 * http://www.mozilla.org/MPL/.                                                                                       *
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


// Similar to std::printf, but uses format() to format arguments and detail::print() to print them.
template<typename T, typename... Args>
void printf(const char *fmt, T value, Args... args)
{
	print(stdout, format(fmt, value, args...));
};

// Similar to std::fprintf, but uses format() to format arguments and detail::print() to print them.
template<typename T, typename... Args>
void fprintf(FILE *file, const char *fmt, T value, Args... args)
{
	print(file, format(fmt, value, args...));
};

void printf(const String &s);

std::optional<String> read_line();


}} // phonometrica::utils

#endif // PHONOMETRICA_PRINT_HPP
