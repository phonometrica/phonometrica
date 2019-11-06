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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cstring>
#include <phon/utils/alloc.hpp>

namespace phonometrica { namespace utils {

void *alloc(intptr_t size)
{
	void *ptr = std::malloc((size_t) size);

	if (!ptr) {
		throw std::bad_alloc();
	}

	return ptr;
}

void *calloc(intptr_t count, intptr_t size)
{
	void *ptr = std::calloc(size_t(count), size_t(size));

	if (!ptr) {
		throw std::bad_alloc();
	}

	return ptr;
}

void *realloc(void *ptr, intptr_t size)
{
	void *new_ptr = std::realloc(ptr, (size_t) size);

	if (!new_ptr) {
		throw std::bad_alloc();
	}

	return new_ptr;
}

void free(void *ptr)
{
	std::free(ptr);
}

}} // namespace phonometrica::utils
