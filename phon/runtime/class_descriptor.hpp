/***********************************************************************************************************************
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
 * Created: 04/06/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: manages classes known at compile time.                                                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CLASS_DESCRIPTOR_HPP
#define PHONOMETRICA_CLASS_DESCRIPTOR_HPP

#include <type_traits>
#include <phon/runtime/definitions.hpp>

namespace phonometrica {
class Class;
}

namespace phonometrica::detail {


// A template to keep track of classes known at compile time. This should not be accessed directly: use
// Class::get<T>() instead.
template<typename T>
struct ClassDescriptor
{
	static Class *get()
	{
		// Class is null while we are bootstrapping the class system. The runtime will check that we have a valid pointer for Class.
		assert(isa || (std::is_same_v<T, Class>));
		return isa;
	}

	static void set(Class *cls)
	{
		assert(isa == nullptr);
		isa = cls;
	}

private:

	static Class *isa;
};

template<class T>
Class *ClassDescriptor<T>::isa = nullptr;

} // namespace phonometrica::detail

#endif // PHONOMETRICA_CLASS_DESCRIPTOR_HPP
