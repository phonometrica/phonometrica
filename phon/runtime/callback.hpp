/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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
 * Purpose: native callback.                                                                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CALLBACK_HPP
#define PHONOMETRICA_CALLBACK_HPP

#include <bitset>
#include <functional>
#include <phon/runtime/variant.hpp>

namespace phonometrica {

class Runtime;

// Flags used to distinguish references and values in function signatures.
static constexpr size_t PARAM_BITSET_SIZE = 64;
using ParamBitset = std::bitset<PARAM_BITSET_SIZE>;


// A native C++ callback.
using NativeCallback = std::function<Variant(Runtime &rt, std::span<Variant> args)>;

} // namespace phonometrica

#endif // PHONOMETRICA_CALLBACK_HPP
