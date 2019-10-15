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

#if PHON_ENABLE_TRACING
#define PHON_TRACE(x) qDebug() << x;
#else
#define PHON_TRACE(x)
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
