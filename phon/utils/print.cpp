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
