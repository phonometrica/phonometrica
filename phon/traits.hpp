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
 * Purpose: type traits.                                                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TRAITS_HPP
#define PHONOMETRICA_TRAITS_HPP

#include <type_traits>
#include <phon/definitions.hpp>

namespace phonometrica {

class String;
class File;
class Regex;
struct Variant;

namespace traits {

// Type that may contain cyclic references
template<typename T> struct maybe_cyclic : std::true_type
{

};


template<> struct maybe_cyclic<String> : std::false_type
{

};

template<> struct maybe_cyclic<File> : std::false_type
{

};

template<> struct maybe_cyclic<Regex> : std::false_type
{

};


//----------------------------------------------------------------------------------------------------------------------

template<typename T> struct is_collectable
{
	static constexpr bool value = !std::is_scalar<T>::value && maybe_cyclic<T>::value;
};

#if 0
template<typename T> struct is_collectable<std::vector<T>>
{
	static constexpr bool value = is_collectable<T>::value;
};

template<typename T> struct is_collectable<std::set<T>>
{
	static constexpr bool value = is_collectable<T>::value;
};

template<typename T, typename U> struct is_collectable<std::unordered_map<T, U>>
{
	static constexpr bool value = is_collectable<T>::value || is_collectable<U>::value;
};
#endif


//----------------------------------------------------------------------------------------------------------------------

template<typename T> struct is_clonable : std::true_type
{

};

template<> struct is_clonable<File> : std::false_type
{

};


//----------------------------------------------------------------------------------------------------------------------

template<typename T> struct is_safely_movable
{
	static constexpr bool value = std::is_trivially_move_constructible<T>::value;
};

template<> struct is_safely_movable<String> : std::true_type
{

};

// FIXME: circumvent error CV2139 with MSVC
//  "an undefined class is not allowed as an argument to compiler intrinsic type trait 'trait'"
template<> struct is_safely_movable<Variant> : std::false_type
{

};


}} // phonometrica::traits

#endif // PHONOMETRICA_TRAITS_HPP
