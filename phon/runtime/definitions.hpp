/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 22/05/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: common definitions.                                                                                        *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_DEFINITIONS_HPP
#define PHONOMETRICA_DEFINITIONS_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <functional>
#include <phon/utils/span.hpp>

#define PHON_UNUSED(x) (void)(x)

#ifdef __GNUC__
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
#else
#define likely(x)       x
#define unlikely(x)     x
#endif

// File extension
#ifndef PHON_FILE_EXTENSION
#	define PHON_FILE_EXTENSION ".phon"
#endif


#ifdef PHON_EMBED_SCRIPTS
#define run_script(runtime, name) runtime.do_string(name##_script)
#define get_script_content(runtime, name) name##_script
#else
#define run_script(rt, name) rt.do_file(Settings::get_std_script(rt, #name))
#define get_script_content(rt, name) File::read_all(Settings::get_std_script(rt, #name))
#endif

namespace phonometrica {

namespace meta {

static constexpr size_t pointer_size = sizeof(void*);
static constexpr bool is_arch32 = (pointer_size == 4);
static constexpr bool is_arch64 = (pointer_size == 8);

} // namespace phonometrica::meta


// Largest and smallest integers that can be safely stored in a double.
static constexpr double largest_integer = 9007199254740992;
static constexpr double smallest_integer = -9007199254740992;

// Forward declarations.
class Object;
class Collectable;

// Callback for the garbage collector.
using GCCallback = std::function<void(Collectable*)>;

} // namespace phonometrica


// If logging is enabled, messages are logged to a file named "phonometrica.log" in the user's home directory.
// Add "#include <wx/log.h>" to files that make use of this macro.
#if defined(PHON_ENABLE_LOGGING) || defined(PHON_DEBUG)
//#define PHON_LOG(...) wxLogDebug(__VA_ARGS__);
#define PHON_LOG(...) fprintf(stderr, __VA_ARGS__);
#else
#define PHON_LOG(...);
#endif

#define PHON_MAX_FORMANTS 10

#endif // PHONOMETRICA_DEFINITIONS_HPP
