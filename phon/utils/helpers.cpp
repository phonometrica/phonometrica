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

#include <ctime>
#include <sstream>
#include <phon/string.hpp>

#if PHON_WINDOWS
#	include <wchar.h>
#	include <windows.h>
#else
#	include <unistd.h>
#	include <cstring>
#endif

#include <phon/utils/helpers.hpp>

namespace phonometrica { namespace utils {

static size_t the_random_seed = 0;

size_t random_seed()
{
	return the_random_seed;
}

void init_random_seed()
{
	srand((unsigned int) time(nullptr));
    the_random_seed = (size_t) rand();
}

FILE *open_file(const String &path, const char *mode)
{
#if PHON_WINDOWS
	auto wpath = path.to_wide();
    auto wmode = String::to_wide(mode);

    return _wfopen(wpath.data(), wmode.data());
#else
	return fopen(path.data(), mode);
#endif
}

FILE *reopen_file(const String &path, const char *mode, FILE *stream)
{
#if PHON_WINDOWS
	auto wpath = path.to_wide();
    auto wmode = String::to_wide(mode);

    return _wfreopen(wpath.data(), wmode.data(), stream);
#else
	return freopen(path.data(), mode, stream);
#endif
}

#if !defined(PHON_ENDIANNES_KNOWN) && !PHON_WINDOWS
bool is_big_endian()
{
	union {
		uint32_t i;
		char c[4];
	} val = {0x01020304};

	return val.c[0] == 1;
}
#endif // check endianness


String new_uuid(size_t len)
{
	// Credits: https://stackoverflow.com/a/440240
	static const char chars[] = "0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	String s(len+1, true);

	for (size_t i = 0; i < len; ++i) {
		char c = chars[rand() % (sizeof(chars) - 1)];
		s.append(c);
	}

	return s;
}

std::string get_version()
{
	std::ostringstream os;
	os << PHON_VERSION_MAJOR << "." << PHON_VERSION_MINOR << "." << PHON_VERSION_MICRO;

	if (PHON_VERSION_DEVEL > 0)
	{
		os << " (devel " << PHON_VERSION_DEVEL << ")";
	}

	return os.str();
}

std::string get_date()
{
	std::ostringstream os;

	if (PHON_RELEASE_DATE_DAY < 10) {
		os << "0";
	}
	os << PHON_RELEASE_DATE_DAY << "/";

	if (PHON_RELEASE_DATE_MONTH < 10) {
		os << "0";
	}
	os << PHON_RELEASE_DATE_MONTH << "/";
	os << PHON_RELEASE_DATE_YEAR;

	return os.str();
}

size_t get_system_memory()
{
	// See: https://stackoverflow.com/a/2513561
#if PHON_WINDOWS
	MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);

    return (size_t) status.ullTotalPhys;
#else
	size_t pages = sysconf(_SC_PHYS_PAGES);
	size_t page_size = sysconf(_SC_PAGE_SIZE);

	return pages * page_size;
#endif
}


}} // namespace::utils
