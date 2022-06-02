/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                      *
 *                                                                                                                    *
 * The contents of this file are subject to the Mozilla Public License Version 2.0 (the "License"); you may not use   *
 * this file except in compliance with the License. You may obtain a copy of the License at                           *
 * http://www.mozilla.org/MPL/.                                                                                       *
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
#include <phon/runtime/traits.hpp>

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
