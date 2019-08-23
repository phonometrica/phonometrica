/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 20/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: low-level memory allocation routines.                                                                     *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_ALLOC_HPP
#define PHONOMETRICA_ALLOC_HPP

#include <cstddef>
#include <stdexcept>
#include <algorithm>
#include <phon/traits.hpp>
#include <phon/definitions.hpp>

namespace phonometrica { namespace utils {


// Allocate a block of uninitialized memory.
void *alloc(intptr_t size);

// Allocate a block of zero-initialized memory.
void *calloc(intptr_t count, intptr_t size);

// Reallocate a block of memory.
void *realloc(void *ptr, intptr_t size);

// Free a block of memory.
void free(void *ptr);


// Allocate a 0-initialized array of `size` items.
template<typename T>
T *allocate(intptr_t size)
{
	return reinterpret_cast<T*>(calloc(size, sizeof(T)));
}

// Reallocate array of items, ensuring that the remaining space is 0 allocated.
template<typename T>
T *reallocate(T *data, intptr_t count, intptr_t capacity)
{
	T *new_data;

	if constexpr (traits::is_safely_movable<T>::value)
	{
		size_t count_nbytes = count * sizeof(T);
		size_t capacity_nbytes = capacity * sizeof(T);
		new_data = reinterpret_cast<T*>(realloc(data, capacity_nbytes));
		auto start = reinterpret_cast<std::byte*>(new_data) + count_nbytes;
		auto end = reinterpret_cast<std::byte*>(new_data) + capacity_nbytes;
		std::fill(start, end, std::byte{0});
	}
	else
	{
		new_data = reinterpret_cast<T*>(utils::calloc(capacity, sizeof(T)));
		std::move(data, data + count, new_data);
		utils::free(data);
	}

	return new_data;
}

}} // namespace phonometrica::utils

#endif // PHONOMETRICA_ALLOC_HPP
