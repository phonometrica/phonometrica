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

#include <ctime>
#include <sstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <phon/string.hpp>

#if PHON_WINDOWS
#	include <wchar.h>
#else
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


std::string new_uuid()
{
    auto uuid = boost::uuids::random_generator()();
    std::ostringstream os;
    os << uuid;

    return os.str();
}

std::string get_version()
{
    std::ostringstream os;
    os << PHON_VERSION_MAJOR << "." << PHON_VERSION_MINOR << "." << PHON_VERSION_MICRO;

    if (PHON_VERSION_NANO > 0)
    {
    	os << " (devel " << PHON_VERSION_NANO << ")";
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

}} // namespace::utils
