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
 * Created: 12/07/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: type metadata.                                                                                             *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_META_HPP
#define PHONOMETRICA_META_HPP

#include <cmath>
#include <phon/runtime/string.hpp>
#include <phon/runtime/traits.hpp>
#include <phon/runtime/class_descriptor.hpp>

namespace phonometrica {
namespace meta {

namespace detail {

String get_class_name_helper(Class *klass);
} // detail

template<class T>
Class *get_class()
{
	return phonometrica::detail::ClassDescriptor<T>::get();
}

template<class T>
String get_class_name()
{
	return detail::get_class_name_helper(get_class<T>());
}

bool is_base_of(const Class *base, const Class *derived);

// Convert type to string.
template<typename T>
String to_string(const T &)
{
	throw error("[Type error] Type % cannot be converted to string", get_class_name<T>());
}

inline String to_string(bool value)
{
	return String::convert(value);
}

inline String to_string(intptr_t value)
{
	return String::convert(value);
}

inline String to_string(double value)
{
	return String::convert(value);
}

inline String to_string(const Array<double> &array)
{
	String s("@[");
	if (array.ndim() > 1) s.append('\n');
	for (intptr_t i = 1; i <= array.nrow(); i++)
	{
		if (array.ndim() > 1) s.append('\t');
		for (intptr_t j = 1; j <= array.ncol(); j++)
		{
			s.append(String::format("%f, ", array(i,j)));
		}
		if (array.ndim() > 1) s.append('\n');
	}
	if (array.ndim() > 1) {
		s.remove_last(", \n");
	}
	else {
		s.remove_last(", ");
	}
	if (array.ndim() > 1) s.append("\n ");
	s.append("]");

	return s;
}
//----------------------------------------------------------------------------------------------------------------------

namespace detail {

template<typename T>
bool equal_helper(const T &v1, const T &v2, std::true_type)
{
	return std::equal_to<T>{}(v1, v2);
}

template<typename T>
bool equal_helper(const T &, const T &, std::false_type)
{
	throw error("[Type error] Values of type % cannot be compared for equality", get_class_name<T>());
}

} // namespace detail

template<class T>
bool equal(const T &v1, const T &v2)
{
	return detail::equal_helper(v1, v2, traits::is_equatable<T>());
}

inline bool equal(double x, double y)
{
	// Comparison method by Christer Ericson. See http://doubletimecollisiondetection.net/blog/?p=89
	double scale = std::max<double>(1.0, std::max<double>(std::fabs(x), std::fabs(y)));
	return std::fabs(x - y) <= std::numeric_limits<double>::epsilon() * scale;
}


//----------------------------------------------------------------------------------------------------------------------

template<class T>
int compare(const T &, const T &)
{
	throw error("[Internal error] compare<T> must be specialized for type %", get_class_name<T>());
}

inline int compare(bool v1, bool v2)
{
	return int(v1) - int(v2);
}

inline int compare(intptr_t v1, intptr_t v2)
{
	if constexpr (sizeof(intptr_t) > sizeof(int))
	{
		auto r = v1 - v2;
		return (r < 0) ? -1 : ((r > 0) ? 1 : 0);
	}
	else
	{
		return v1 - v2;
	}
}

inline int compare(double v1, double v2)
{
	if (equal(v1, v2))
	{
		return 0;
	}
	else if ((v1 - v2) < 0)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

//----------------------------------------------------------------------------------------------------------------------

namespace detail {

template<typename T>
size_t hash_helper(const T &value, std::true_type)
{
	return std::hash<T>{}(value);
}

template<typename T>
size_t hash_helper(const T &, std::false_type)
{
	throw error("[Type error] Type % is not hashable", get_class_name<T>());
}
} // namespace detail


// Credit: https://gist.github.com/badboy/6267743
static inline size_t hash(uint64_t n)
{
	n = (~n) + (n << 21); // key = (key << 21) - key - 1;
	n = n ^ (n >> 24);
	n = (n + (n << 3)) + (n << 8); // key * 265
	n = n ^ (n >> 14);
	n = (n + (n << 2)) + (n << 4); // key * 21
	n = n ^ (n >> 28);
	n = n + (n << 31);

	if constexpr (sizeof(size_t) == 8)
	{
		return n;
	}
	else
	{
		union Bits {
			Bits(uint64_t i) : bits(i) { }
			uint64_t bits; 
			struct { uint32_t x, y; }; 
		} u(n);
		return u.x + u.y;
	}
}


//----------------------------------------------------------------------------------------------------------------------

namespace detail {

template<class T>
void traverse(typename std::enable_if<traits::is_collectable<T>::value, T>::type &, const GCCallback &)
{
	throw error("[Internal error] traversal is not implemented for type %", get_class_name<T>());
}

template<class T>
void traverse(typename std::enable_if<!traits::is_collectable<T>::value, T>::type &, const GCCallback &)
{
	// Nothing to do.
}

} // namespace detail


// Types which can contain cyclic references must specialize or overload this function template. (See how list and table do it.)
template<class T>
void traverse(T &value, const GCCallback &callback)
{
	detail::traverse<T>(value, callback);
}


}} // namespace phonometrica::meta

#endif // PHONOMETRICA_META_HPP
