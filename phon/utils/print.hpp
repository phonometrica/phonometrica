/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
 *                                                                                                                     *
 * Created: 20/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: cross-platform printing routines.                                                                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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
