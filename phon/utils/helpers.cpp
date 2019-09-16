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
    	os << " (test " << PHON_VERSION_NANO << ")";
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
