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
 * Created: 20/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: type traits.                                                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TRAITS_HPP
#define PHONOMETRICA_TRAITS_HPP

#include <type_traits>
#include <phon/runtime/definitions.hpp>

namespace phonometrica {

class Object;
class String;
class File;
class Regex;
class Variant;
class Iterator;
class ListIterator;
class TableIterator;
class StringIterator;
class FileIterator;
class RegexIterator;
template<class T> class Array;

// Dummy base class for Float and Integer
class Number {};


namespace traits {

// Type that may contain cyclic references
template<typename T> struct maybe_cyclic : std::true_type
{

};

#define NON_CYCLIC(T) template<> struct maybe_cyclic<T> : std::false_type { };

NON_CYCLIC(String);
NON_CYCLIC(File);
NON_CYCLIC(Regex);
NON_CYCLIC(Iterator);
NON_CYCLIC(ListIterator);
NON_CYCLIC(TableIterator);
NON_CYCLIC(StringIterator);
NON_CYCLIC(FileIterator);
NON_CYCLIC(RegexIterator);
NON_CYCLIC(Array<double>);

#undef NON_CYCLIC


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

template<typename T> struct is_clonable : std::is_copy_constructible<T>
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


//----------------------------------------------------------------------------------------------------------------------

template<typename T>
struct is_boxed : std::true_type { };

template<>
struct is_boxed<bool> : std::false_type { };

template<>
struct is_boxed<intptr_t> : std::false_type { };

template<>
struct is_boxed<double> : std::false_type { };

template<>
struct is_boxed<Number> : std::false_type { };

template<>
struct is_boxed<String> : std::false_type { };

// Abstract type, but we need to create a class for it since it is the base of all classes.
template<>
struct is_boxed<Object> : std::false_type { };

//----------------------------------------------------------------------------------------------------------------------

// Credits to: https://stackoverflow.com/a/39348287
namespace detail {
template<class X, class Y, class Op>
struct op_valid_impl
{
	template<class U, class L, class R>
	static auto test(int) -> decltype(std::declval<U>()(std::declval<L>(), std::declval<R>()),
			void(), std::true_type());

	template<class U, class L, class R>
	static auto test(...) -> std::false_type;

	using type = decltype(test<Op, X, Y>(0));

};
} // namespace detail

template<class X, class Y, class Op>
using is_operator_valid = typename detail::op_valid_impl<X, Y, Op>::type;


template<typename T>
using is_equatable = is_operator_valid<T,T,std::equal_to<>>;

template<typename T>
using is_hashable = std::is_default_constructible<std::hash<T>>;

//----------------------------------------------------------------------------------------------------------------------

template<typename T>
using bare_type = std::remove_cv<typename std::remove_reference<T>::type>;


}} // phonometrica::traits

#endif // PHONOMETRICA_TRAITS_HPP
