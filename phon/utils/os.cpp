/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019-2022 Julien Eychenne <jeychenne@gmail.com>                                                      *
 *                                                                                                                    *
 * The contents of this file are subject to the Mozilla Public License Version 2.0 (the "License"); you may not use   *
 * this file except in compliance with the License. You may obtain a copy of the License at                           *
 * http://www.mozilla.org/MPL/.                                                                                       *
 *                                                                                                                    *
 * Created: 20/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

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
