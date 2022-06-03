/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019-2022 Julien Eychenne <jeychenne@gmail.com>                                                      *
 *                                                                                                                    *
 * The contents of this file are subject to the Mozilla Public License Version 2.0 (the "License"); you may not use   *
 * this file except in compliance with the License. You may obtain a copy of the License at                           *
 * http://www.mozilla.org/MPL/.                                                                                       *
 *                                                                                                                    *
 * Created: 20/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

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
