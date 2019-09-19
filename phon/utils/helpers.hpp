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

}} // namespace::utils

#endif // PHONOMETRICA_HELPERS_HPP
