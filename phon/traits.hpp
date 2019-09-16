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
