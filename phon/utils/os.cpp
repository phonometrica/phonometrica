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

#include <cstring>
#include <phon/utils/os.hpp>

#if PHON_WINDOWS
#include <Windows.h>
#else
#	include <cerrno>
#endif

namespace phonometrica { namespace utils {

String error_message()
{
#if PHON_WINDOWS
	DWORD err = GetLastError();
	LPTSTR msg = NULL;

	FormatMessage(
	   FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS,
	   NULL,
	   err,
	   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	   (LPTSTR)&msg,
	   0,
	   NULL);

	if (msg != NULL) {
		auto result = String(msg, intptr_t(wcslen(msg)));
		LocalFree((HLOCAL)msg);

		return result;
	}
	else {
		return String();
	}
#else
	return std::strerror(errno);
#endif
}

}} // namespace phonometrica::utils
