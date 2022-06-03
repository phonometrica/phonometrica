/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019-2022 Julien Eychenne <jeychenne@gmail.com>                                                      *
 *                                                                                                                    *
 * The contents of this file are subject to the Mozilla Public License Version 2.0 (the "License"); you may not use   *
 * this file except in compliance with the License. You may obtain a copy of the License at                           *
 * http://www.mozilla.org/MPL/.                                                                                       *
 *                                                                                                                    *
 * Created: 12/04/2021                                                                                                *
 *                                                                                                                    *
 * Purpose: an array view similar to std::span in C++20, but in base 1 (similar to Array<T>).                         *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_SLICE_HPP
#define PHONOMETRICA_SLICE_HPP

#include <phon/array.hpp>

namespace phonometrica {

template<class T>
class Slice final
{
public:

	using size_type = intptr_t; // non-standard-compliant, since the standard mandates an unsigned type.
	using difference_type = intptr_t;
	using value_type = T;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	Slice() = default;

	Slice(const value_type *data, size_type size) :
		m_data(data), m_size(size) { }

	Slice(const_iterator start, const_iterator end) :
		m_data(start), m_size(size_type(end - start)) { }

	Slice(const Array<T> &array) :
		m_data(array.data()), m_size(array.size()) { }

	Slice(const Slice &other) = default;

	Slice(Slice &&other) noexcept = default;

	~Slice() = default;

	Slice &operator=(Slice<T> other)
	{
		m_data = other.data();
		m_size = other.size();

		return *this;
	}

	const value_type *data() const { return m_data; }

	size_type size() const { return m_size; }

	bool empty() const noexcept { return size() == 0; }

	reference operator[](size_type pos) noexcept
	{
		assert(pos > 0 && pos <= this->size());
		return m_data[--pos];
	}

	const_reference operator[](size_type pos) const noexcept
	{
		assert(pos > 0 && pos <= this->size());
		return m_data[--pos];
	}

	reference at(size_type i)
	{
		return m_data[to_base0(i)];
	}

	const_reference at(size_type i) const
	{
		return m_data[to_base0(i)];
	}

	// Forward iterators.
	iterator begin() noexcept { return m_data; }
	const_iterator begin() const noexcept { return m_data; }
	const_iterator cbegin() const noexcept { return begin(); }
	iterator end() noexcept { return m_data + size(); }
	const_iterator end() const noexcept { return m_data + size(); }
	const_iterator cend() const noexcept { return end(); }

	// Reverse iterators.
	reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
	const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
	const_reverse_iterator crbegin() const noexcept { return rbegin(); }

	reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
	const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
	const_reverse_iterator crend() const noexcept { return rend(); }


private:

	size_type to_base0(size_type i) const
	{
		if (i > 0 && i <= m_size) {
			return i - 1;
		}
		if (i >= -m_size && i < 0) {
			return m_size + i;
		}

		throw error("Index % out of range in span with length %", m_size);
	}

	T *m_data = nullptr;
	size_type m_size = 0;
};

} // namespace phonometrica

#endif // PHONOMETRICA_SLICE_HPP
