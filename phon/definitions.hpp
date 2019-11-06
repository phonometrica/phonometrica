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
 * Purpose: common definitions.                                                                                        *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_DEFINITIONS_HPP
#define PHONOMETRICA_DEFINITIONS_HPP

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <limits>

/* Detect architecture (either 32 bit or 64 bit) */
#ifdef _WIN64
	#define PHON_ARCH64
#elif defined(_WIN32)
	#define PHON_ARCH32
#elif defined(__i386__)
	#define PHON_ARCH32
#elif defined(__x86_64__)
	#define PHON_ARCH64
#elif defined(__GNUC__)
#ifdef __LP64__
		#define PHON_ARCH64
	#else
		#define PHON_ARCH32
	#endif
#else
	#error "Unsupported platform."
#endif

// If logging is enabled, messages are logged to a file named "phonometrica.log" in the user's home directory.
// Add "#include <QDebug>" to files that make use of this macro.
#ifdef PHON_ENABLE_LOGGING
#define PHON_LOG(x) qDebug() << x;
#else
#define PHON_LOG(x);
#endif

#define PHON_MAC_SIDEBAR_COLOR "#E5E7EC"

#define PHON_EXT_ANNOTATION ".phon-annot"

#define PHON_MAX_FORMANTS 10

namespace phonometrica {

namespace meta {
constexpr size_t pointer_size = sizeof(void*);
static constexpr bool is_arch32 = (pointer_size == 4);
static constexpr bool is_arch64 = (pointer_size == 8);

} // namespace meta

// Color for the garbage collector. Objects that are acyclic (i.e. contain no cyclic reference)
// are green. Base types such as string and regex are acyclic because there is no way they
// can store a reference to themselves. Collections (list, table, etc.) are considered cyclic
// and are therefore candidates for GC.
enum class GCColor
{
	Green,      // object which is not collectable
	Black,      // Assumed to be alive
	Grey,       // Possible member of a GC cycle
	White,      // Possibly dead
	Purple      // Root candidate for a GC cycle
};

// Type for interned strings in the scripting engine.
typedef int32_t literal;
static const literal unknown_symbol = (std::numeric_limits<literal>::min)();

} // namespace phonometrica



#endif // PHONOMETRICA_DEFINITIONS_HPP
