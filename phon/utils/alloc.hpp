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
 * Purpose: low-level memory allocation routines.                                                                      *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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
