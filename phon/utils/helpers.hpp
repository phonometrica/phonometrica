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
 * Purpose: helper routines.                                                                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_HELPERS_HPP
#define PHONOMETRICA_HELPERS_HPP

#include <cstdio>
#include <string>
#include <phon/definitions.hpp>

/* Use byte swapping primitives if available, otherwise default to generic versions */
#if defined(_WIN32) && !defined(PHON_MINGW)
#include <intrin.h>
#   define PHON_BYTESWAP16(x) _byteswap_ushort(x)
#   define PHON_BYTESWAP32(x) _byteswap_ulong(x)
#   define PHON_HAS_BYTESWAP
#elif defined(__APPLE__)
#    include <libkern/OSByteOrder.h>
#   define PHON_BYTESWAP16(x) OSSwapInt16(x)
#   define PHON_BYTESWAP32(x) OSSwapInt32(x)
#   define PHON_HAS_BYTESWAP
#   define PHON_ENDIANNES_KNOWN
#elif (defined(__GNUC__) || defined(__GNUG__)) && !defined(PHON_MINGW)
#    include <byteswap.h>

#   define PHON_BYTESWAP16(x) __bswap_16(x)
#   define PHON_BYTESWAP32(x) __bswap_32(x)
#   define PHON_HAS_BYTESWAP
#   define PHON_ENDIANNES_KNOWN
#else
#   define PHON_BYTESWAP16(x) ((x >> 8) | (x << 8))
#   define PHON_BYTESWAP32(x) \
    ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
    (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))
#endif

namespace phonometrica
{
    class String;
}


namespace phonometrica { namespace utils {



// Check whether the platform is big endian (most likely false)
// Use compile-time check if possible.
#if PHON_WINDOWS
static inline bool is_big_endian()
{
	return false;
}

#elif defined(PHON_ENDIANNES_KNOWN)

static inline bool is_big_endian()
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	return true;
#else
	return false;
#endif
}

#else
bool is_big_endian();
#endif // end check endianness




void init_random_seed();

size_t random_seed();

static inline
intptr_t next_capacity(intptr_t n)
{
	assert(n != 0);
	return (n < 32) ? (n << 1) : (n + (n >> 1));
}

static inline
intptr_t find_capacity(intptr_t requested, intptr_t capacity = 8)
{
	// Find a capacity which is at least what is requested.
	while (capacity < requested) capacity = utils::next_capacity(capacity);

	return capacity;
}

std::string new_uuid();

std::string get_version();

std::string get_date();


FILE *open_file(const String &path, const char *mode);

FILE *reopen_file(const String &path, const char *mode, FILE *stream);


template<class T>
T minimum(T x, T y, T z)
{
	return (std::min)((std::min)(x, y), z);
}


template<class T>
T maximum(T x, T y, T z)
{
	return (std::max)((std::max)(x, y), z);
}

}} // namespace::utils

#endif // PHONOMETRICA_HELPERS_HPP
