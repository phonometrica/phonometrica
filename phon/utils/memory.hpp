/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * Purpose: convenience templates to deal with smart pointers.                                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_MEMORY_HPP
#define PHONOMETRICA_MEMORY_HPP

#include <memory>

namespace phonometrica {

// Get a downcasted raw pointer from a shared pointer.
template<class Derived, class Base>
Derived *raw_cast(const std::shared_ptr<Base> &ptr)
{
	static_assert(std::is_base_of<std::remove_cv_t<Base> , std::remove_cv_t<Derived>>::value, "Base must be a base of Derived");
	return static_cast<Derived*>(ptr.get());
};

// Get a downcasted shared pointer from another shared pointer.
template<class Derived, class Base>
std::shared_ptr<Derived> downcast(const std::shared_ptr<Base> &ptr)
{
	static_assert(std::is_base_of<Base,Derived>::value, "Base must be a base of Derived");
	return std::static_pointer_cast<Derived>(ptr);
};

template<class Base, class Derived>
std::shared_ptr<Base> upcast(const std::shared_ptr<Derived> &ptr)
{
	static_assert(std::is_base_of<Base,Derived>::value, "Base must be a base of Derived");
	return std::static_pointer_cast<Base>(ptr);
};


} // namespace phonometrica

#endif // PHONOMETRICA_MEMORY_HPP
