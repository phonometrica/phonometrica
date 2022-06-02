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
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

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
