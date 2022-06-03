/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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
 * Purpose: One multidimensional array to rule them all. Indices are 1-based and can be negative (starting from the    *
 * end). If the array has a single dimension, it can be appended to and grows dynamically like an std::vector. If it   *
 * has 2 or more dimensions, its size is fixed at construction time.                                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_ARRAY_HPP
#define PHONOMETRICA_ARRAY_HPP

#include <algorithm>
#include <vector>
#include <type_traits>
#include <phon/error.hpp>
#include <phon/runtime/traits.hpp>
#include <phon/utils/alloc.hpp>
#include <phon/utils/helpers.hpp>
#include <phon/utils/span.hpp>

namespace phonometrica {

template<class T>
class Array final
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
	using index_sequence = std::vector<size_type>;

	static constexpr bool has_scalar_type = std::is_scalar<value_type>::value;
	static constexpr bool has_safely_movable_value_type = traits::is_safely_movable<value_type>::value;


	// Empty 1 dimension array.
	Array() noexcept
	{
		m_ndim = 1;
		m_size = 0;
		m_data = nullptr;
		m_dim.d1.capacity = 0;
	}

	static Array from_memory(T *malloced, intptr_t size)
	{
		Array self;
		self.m_data = malloced;
		self.m_size = size;
		self.m_dim.d1.capacity = size;
		self.m_ndim = 1;

		return self;
	}

	// Copy constructor.
	Array(const Array &other)
	{
		m_ndim = other.ndim();
		m_size = other.size();

		switch (ndim())
		{
			case 1:
			{
				m_dim.d1.capacity = other.m_dim.d1.capacity;
				break;
			}
			case 2:
			{
				m_dim.d2.nrow = other.nrow();
				m_dim.d2.ncol = other.ncol();
				break;
			}
			default:
			{
				alloc_dims(ndim());
				std::copy(other.shape(), other.shape() + ndim(), this->shape());
				std::copy(other.jumps(), other.jumps() + ndim(), this->jumps());
			}
		}

		auto count = (ndim() == 1) ? capacity() : size();
		m_data = utils::allocate<value_type>(count);
		std::copy(other.begin(), other.end(), this->begin());
	}

	// Move constructor.
	Array(Array &&other) noexcept
	{
        m_ndim = other.ndim();
        m_size = other.size();

        switch (ndim())
        {
            case 1:
            {
                m_dim.d1.capacity = other.m_dim.d1.capacity;
                break;
            }
            case 2:
            {
                m_dim.d2.nrow = other.nrow();
                m_dim.d2.ncol = other.ncol();
                break;
            }
            default:
            {
                m_dim.dx.jumps = other.m_dim.dx.jumps;
                m_dim.dx.shape = other.m_dim.dx.shape;
            }
        }

        m_data = other.m_data;
        other.zero();
	}

	// Empty 1 dimension array with a given capacity.
	explicit Array(size_type requested)
	{
		size_type capacity = utils::find_capacity(requested);
		m_ndim = 1;
		m_size = 0;
		m_dim.d1.capacity = capacity;
		m_data = utils::allocate<value_type>(capacity);
	}

	Array(std::span<T> span) :
		Array(size_type(span.size()))
	{
		m_size = size_type(span.size());
		std::copy(span.begin(), span.end(), this->begin());
	}

	// 1 dimension array filled with a given value.
	Array(size_type count, const_reference value) :
		Array(count)
	{
		m_size = count;
		copy_construct_n(begin(), std::size_t(count), value);
	}

	Array(const_iterator from, size_type size) :
		Array(size)
	{
		m_size = size;
		std::copy(from, from + size, m_data);
	}

	Array(const_iterator start, const_iterator end) :
		Array(start, intptr_t(end - start))
	{ }

	// 1 dimension array constructed from a list of values.
	Array(std::initializer_list<value_type> items) :
		Array(size_type(items.size()))
	{
		m_size = size_type(items.size());
		std::copy(items.begin(), items.end(), this->begin());
	}

	// Default-initialized matrix.
	Array(size_type nrow, size_type ncol)
	{
		size_type size = nrow * ncol;

		m_ndim = 2;
		m_size = size;
		m_dim.d2.nrow = nrow;
		m_dim.d2.ncol = ncol;
		alloc_data_ndim(size);
	}

	// Default-initialized matrix with an explicit value.
	Array(size_type nrow, size_type ncol, const_reference value)
	{
		size_type size = nrow * ncol;

		m_ndim = 2;
		m_size = size;
		m_dim.d2.nrow = nrow;
		m_dim.d2.ncol = ncol;
        alloc_data_ndim(size, value);
	}

	// Construct and default initialize an N-dimension array (with N > 2).
	explicit Array(const index_sequence &indexes) :
		Array(indexes, value_type())
	{

	}

	// Construct and default initialize an N-dimension array (with N > 2) with an explicit value.
	Array(const index_sequence &indexes, const_reference value)
	{
		assert(indexes.size() > 2);
		m_ndim = size_type(indexes.size());
		alloc_dims(ndim());
		init_shape(indexes);
		init_jumps();

		// Calculate size.
		m_size = 1;

		for (size_type i = 0; i < ndim(); ++i) {
			m_size *= this->shape(i);
		}

		alloc_data_ndim(size(), value);
	}

	// Destructor.
	~Array() noexcept
	{
		if (m_data)
		{
			for (auto &value : *this) {
				value.~value_type();
			}

			utils::free(m_data);

			if (ndim() > 2)
			{
				utils::free(this->shape());
				utils::free(this->jumps());
			}
		}
	}

	Array &operator=(Array other) noexcept
	{
		swap(other);
		return *this;
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

	iterator iter(intptr_t i) { return begin() + to_base0(i); }

	const_iterator iter(intptr_t i) const { return begin() + to_base0(i); }

	iterator iter(intptr_t i, intptr_t j) { return begin() + d2_to_base0(i, j); }

	const_iterator iter(intptr_t i, intptr_t j) const { return begin() + d2_to_base0(i, j); }

	size_type ndim() const noexcept { return m_ndim; }

	size_type size() const noexcept { return m_size; }

	size_type max_size() const noexcept { return (std::numeric_limits<size_type>::max)(); }

	size_type capacity() const
	{
		assert(ndim() == 1);
		return m_dim.d1.capacity;
	}

	size_type dim_size(size_type n) const
	{
		switch (ndim())
		{
			case 1:
				if (n == 1) { return m_size; }
				break;
			case 2:
				if (n == 1) { return nrow(); }
				else if (n == 2) { return ncol(); }
				break;
			default:
				if (n >= 1 && n <= ndim()) { return shape(--n); }
		}

		throw error("Cannot access dimension % in array with % dimensions", n, ndim());
	}

	bool empty() const noexcept { return size() == 0; }

	size_type nrow() const noexcept
	{
		if (ndim() == 1) return size();
		assert(ndim() == 2);
		return m_dim.d2.nrow;
	}

	size_type ncol() const noexcept
	{
		if (ndim() == 1) return 1;
		assert(ndim() == 2);
		return m_dim.d2.ncol;
	}

	const value_type *data() const noexcept { return m_data; }

	value_type *data() noexcept { return m_data; }

	void reserve(intptr_t capacity)
	{
		expand(capacity);
	}

	bool operator==(const Array &other) const
	{
		if (this == &other)
			return true;

		if (this->size() != other.size() || this->ndim() != other.ndim())
			return false;

		for (intptr_t i = 0; i < this->size(); i++)
		{
			if (this->m_data[i] != other.m_data[i])
				return false;
		}

		return true;
	}

	void swap(Array &other) noexcept
	{
		std::swap(m_dim, other.m_dim);
		std::swap(m_size, other.m_size);
		std::swap(m_ndim, other.m_ndim);
		std::swap(m_data, other.m_data);
	}

	// Only accepts a positive index in base 1. No bound checking is performed.

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

	reference operator()(size_type pos) noexcept
	{
		return (*this)[pos];
	}

	const_reference operator()(size_type pos) const noexcept
	{
		return (*this)[pos];
	}

	reference operator()(size_type row, size_type col)
	{
		return m_data[d2_to_base0(row, col)];
	}

	const_reference operator()(size_type row, size_type col) const
	{
		return m_data[d2_to_base0(row, col)];
	}

	reference operator()(const index_sequence &indexes)
	{
		assert(ndim() > 2);
		return m_data[to_base0(indexes)];
	}

	const_reference operator()(const index_sequence &indexes) const
	{
		assert(ndim() > 2);
		return m_data[to_base0(indexes)];
	}

	// Access element with a positive (1-based) or negative index. Bounds are checked.

	reference at(size_type i)
	{
		return m_data[to_base0(i)];
	}

	const_reference at(size_type i) const
	{
		return m_data[to_base0(i)];
	}

	reference at(size_type row, size_type col)
	{
		auto i = to_base0(row, nrow());
		auto j = to_base0(col, ncol());
		auto pos = j * nrow() + i;

		return m_data[pos];
	}

	const_reference at(size_type row, size_type col) const
	{
		auto i = to_base0(row, nrow());
		auto j = to_base0(col, ncol());
		auto pos = j * nrow() + i;

		return m_data[pos];
	}

	// Note that the index sequence is mutated in place by converting all indexes to positive 1-based values.
	reference at(index_sequence &indexes)
	{
		normalize_indexes(indexes);
		return (*this)(indexes);
	}

	const_reference at(index_sequence &indexes) const
	{
		normalize_indexes(indexes);
		return (*this)(indexes);
	}

	reference first() { return m_data[0]; }
	const_reference first() const { return m_data[0]; }

	reference last() { return m_data[m_size - 1]; }
	const_reference last() const { return m_data[m_size - 1]; }

	// For compatibility with std::vector.
	reference front() { return first(); }
	const_reference front() const { return first(); }

	reference back() { return last(); }
	const_reference back() const { return last(); }

	// For compatibility with std::vector.
	void push_back(const_reference value)
	{
		append(value);
	}

	void push_back(value_type &&value)
	{
		append(std::move(value));
	}

	void pop_back()
	{
		take_last();
	}

	template <class... Args>
	void emplace_back(Args&&... args)
	{
		expand(size() + 1);
		new (&m_data[m_size++]) value_type(std::forward<Args>(args)...);
	}

	void append(const_reference value)
	{
		expand(size() + 1);
		new (&m_data[m_size++]) value_type(value);
	}

	void append(value_type &&value)
	{
		expand(size() + 1);
		new (&m_data[m_size++]) value_type(std::move(value));
	}

	void append(const Array &other)
	{
		auto new_size = size() + other.size();
		expand(new_size);
		std::copy(other.begin(), other.end(), this->end());
		m_size = new_size;
	}

	void append(std::initializer_list<value_type> values)
	{
		auto new_size = size() + values.size();
		expand(new_size);
		std::copy(values.begin(), values.end(), this->end());
		m_size = new_size;
	}

	void prepend(const_reference value)
	{
		insert(begin(), value);
	}

	void prepend(value_type &&value)
	{
		insert(begin(), std::move(value));
	}

	void insert(intptr_t pos, const_reference value)
	{
		if (pos > size())
		{
			append(value);
		}
		else
		{
			auto it = begin() + to_base0(pos);
			insert(it, value);
		}
	}

	void insert(const_iterator pos, const_reference value)
	{
		size_t offset = pos - begin();
		if (expand(size() + 1)) pos = begin() + offset;
		std::move_backward(cast(pos), end(), end() + 1);
		new (cast(pos)) value_type(value);
		m_size++;
	}

	void insert(intptr_t pos, value_type &&value)
	{
		if (pos > size())
		{
			append(std::move(value));
		}
		else
		{
			auto it = begin() + to_base0(pos);
			insert(it, std::move(value));
		}
	}

	iterator insert(const_iterator pos, value_type &&value)
	{
		size_t offset = pos - begin();
		if (expand(size() + 1)) pos = begin() + offset;
		std::move_backward(cast(pos), end(), end() + 1);

		new (cast(pos)) value_type(std::move(value));
		m_size++;

		return cast(pos);
	}

	template<class InputIterator>
	void insert(size_type pos, InputIterator first, InputIterator last)
	{
		auto it = begin() + to_base0(pos);
		insert(it, first, last);
	}

	template<class InputIterator>
	iterator insert(const_iterator pos, InputIterator first, InputIterator last)
	{
		size_t offset = pos - begin();
		auto range_size = size_type(last - first);
		if (expand(size() + range_size)) pos = begin() + offset;
		std::move_backward(cast(pos), end(), end() + range_size);

		for (auto it = first; it != last; it++) {
			new (cast(pos)) value_type(*it);
		}

		m_size += range_size;

		return cast(pos);
	}

	void insert(size_type pos, const Array &values)
	{
		auto it = begin() + to_base0(pos);
		insert(it, values);
	}

	iterator insert(const_iterator pos, const Array &values)
	{
		return insert(pos, values.begin(), values.end());
	}

	void insert(size_type pos, std::initializer_list<value_type> values)
	{
		auto it = begin() + to_base0(pos);
		insert(it, values);
	}

	iterator insert(const_iterator pos, std::initializer_list<value_type> values)
	{
		return insert(pos, values.begin(), values.end());
	}


	template <class... Args>
	iterator emplace(intptr_t pos, Args&&... args)
	{
		auto it = begin() + to_base0(pos);
		return emplace(pos, std::forward<Args>(args)...);
	}

	template <class... Args>
	iterator emplace(const_iterator pos, Args&&... args)
	{
		size_t offset = pos - begin();
		if (expand(size() + 1)) pos = begin() + offset;
		std::move_backward(cast(pos), end(), end() + 1);
		new (cast(pos)) value_type(std::forward<Args>(args)...);

		return cast(pos);
	}

	// Remove all the occurrences of `value` in the array.
	void remove(const_reference value)
	{
		for (intptr_t i = this->size(); i-- > 0; )
		{
			if (m_data[i] == value)
			{
				remove_at(begin() + i);
			}
		}
	}

	// Remove the leftmost occurrence of `value` in the array.
	void remove_first(const_reference value)
	{
		for (auto it = begin(); it != end(); it++)
		{
			if (*it == value)
			{
				remove_at(it);
				return;
			}
		}
	}

	// Remove the rightmost occurrence of `value` in the array.
	void remove_last(const_reference value)
	{
		for (auto i = this->size(); i > 0; i--)
		{
			if (m_data[i-1] == value)
			{
				remove_at(i);
				return;
			}
		}
	}

	// Remove value at a given position.
	void remove_at(intptr_t pos)
	{
		auto it = begin() + to_base0(pos);
		remove_at(it);
	}

	void remove_at(const_iterator pos)
	{
		assert(ndim() == 1);
		std::move(cast(pos + 1), end(), cast(pos));
		m_size--;
	}

	// Remove a range of items.
	void remove(intptr_t pos, intptr_t count)
	{
		auto from = begin() + to_base0(pos);
		auto to = from + count;

		return remove(from, to);
	}

	void remove(const_iterator from, const_iterator to)
	{
		std::move(to, end(), cast(from));
		m_size -= to - from;
	}

	// Remove n last items.
	void drop(intptr_t n)
	{
		assert(ndim() == 1);
		assert(n >= 0);

		for (auto it = end() - n; it != end(); it++){
			(*it).~value_type();
		}
		m_size -= n;
	}

	// Remove and return the first value in the array.
	value_type take_first()
	{
		value_type tmp(std::move(first()));
		remove_at(begin());

		return tmp;
	}

	// Remove and return the last value in the array.
	value_type take_last()
	{
		value_type tmp(std::move(last()));
		remove_at(end() - 1);

		return tmp;
	}

	T take_at(intptr_t pos)
	{
		auto it = begin() + to_base0(pos);
		return take_at(it);
	}

	T take_at(const_iterator pos)
	{
		assert(ndim() == 1);
		T value(std::move(*pos));
		remove_at(pos);

		return value;
	}

	// Remove the first value in the array.
	void pop_first()
	{
		remove_at(begin());
	}

	// Remove the last value in the array.
	void pop_last()
	{
		m_data[--m_size].~value_type();
	}

	void clear()
	{
		assert(ndim() == 1);
		if (!this->empty())
		{
			auto ptr = m_data + m_size;
			while (--ptr >= m_data) {
				ptr->~value_type();
			}
			m_size = 0;
		}
	}

	bool contains(const_reference value) const
	{
		for (auto &v : *this)
		{
			if (v == value) {
				return true;
			}
		}

		return false;
	}

	bool starts_with(const_reference value) const
	{
		return !this->empty() && first() == value;
	}

	bool ends_with(const_reference value) const
	{
		return !this->empty() && last() == value;
	}

	size_type find(const_reference value, size_type from = 1) const
	{
		if (empty()) return 0; // not found
		auto start = begin() + to_base0(from);
		auto result = find(value, start);

		return (result == end()) ? 0 : size_type(result - begin() + 1);
	}

	const_iterator find(const_reference value, const_iterator from) const
	{
		return std::find(from, end(), value);
	}

	// Find index of value starting from the end or from a negative index.
	intptr_t rfind(const_reference value, intptr_t from = -1) const
	{
		assert(from < 0);
		if (empty()) return 0; // not found
		auto start = rbegin() - from - 1;
		auto result = rfind(value, start);

		if (result == rbegin()) return 0;
		auto rpos = intptr_t( result - rbegin());
		return size() - rpos;
	}

	const_reverse_iterator rfind(const_reference value, const_reverse_iterator from) const
	{
		return std::find(from, rend(), value);
	}

	void resize(size_type new_size)
	{
		assert(ndim() == 1);

		if (new_size < size())
		{
			for (auto it = begin() + new_size; it != end(); it++)
			{
				it->~value_type();
			}
		}
		else if (new_size > size())
		{
			expand(new_size);
			copy_construct(begin() + size(), begin() + new_size, value_type());
		}

		m_size = new_size;
	}

	operator std::span<T>()
	{
		return { data(), size() };
	}

	// Convert index for 1-dimension arrays.
	size_type to_base0(size_type i) const
	{
		auto len = this->size();

		if (i > 0 && i <= len) {
			return --i;
		}
		if (i >= -len && i < 0) {
			return len + i;
		}

		throw error("Index % out of range in array containing % items", i, len);
	}

	void check_dim(const Array &other) const
	{
		if (this->ndim() != other.ndim()) {
			throw error("Arrays have different dimensions");
		}
		if (this->size() != other.size()) {
			throw error("Arrays have different sizes");
		}

		switch (ndim())
		{
			case 1:
				break;
			case 2:
				if (this->nrow() != other.nrow() || this->ncol() != other.ncol()) {
					throw error("Array have different shapes");
				}
				break;
			default:
				for (intptr_t i = 0; i < ndim(); i++)
				{
					if (m_dim.dx.shape[i] != other.m_dim.dx.shape[i]) {
						throw error("Array have different shapes");
					}
				}
		}
	}

	Array<T> slice(intptr_t from, intptr_t to, intptr_t step = 1)
	{
		Array<T> result(to-from+1, value_type());
		auto out = result.begin();
		auto in = this->begin() + from - 1;

		for (intptr_t i = from; i <= to; i += step) {
			*out++ = *in++;
		}

		return result;
	}

	Array<T> slice(intptr_t i1, intptr_t i2, intptr_t j1, intptr_t j2, intptr_t istep = 1, intptr_t jstep=1)
	{
		Array<T> result(i2-i1+1, j2-j1+1, value_type());
		intptr_t ii = 1;

		for (intptr_t i = i1; i <= i2; i += istep)
		{
			intptr_t jj = 1;
			for (intptr_t j = j1; j <= j2; j += jstep)
			{
				result(ii,jj) = (*this)(i,j);
				jj++;
			}
			ii++;
		}

		return result;
	}

private:

	const size_type *shape() const noexcept { return m_dim.dx.shape; }
	size_type *shape() noexcept { return m_dim.dx.shape; }

	const size_type *jumps() const noexcept { return m_dim.dx.jumps; }
	size_type *jumps() noexcept { return m_dim.dx.jumps; }

	// Convert index for N-dimension arrays.
	intptr_t to_base0(const index_sequence & indexes)
	{
		intptr_t i = 0;

		for (size_type k = size_type(indexes.size()) - 1; k >= 0; --k)
		{
			i += indexes[k] * jump(k);
		}

		return i;
	}


	iterator cast(const_iterator it) { return const_cast<iterator>(it); }

	// Convert index for multidimensional arrays.
	intptr_t to_base1(intptr_t i) const
	{
		auto len = this->size();

		if (i > 0 && i <= len) {
			return i;
		}
		if (i >= -len && i < 0) {
			return len + i;
		}

		throw error("Index % out of range in array with size %", i, len);
	}

	size_type to_base0(size_type i, size_type len) const
	{
		if (i > 0 && i <= len) {
			return i - 1;
		}
		if (i >= -len && i < 0) {
			return len + i;
		}

		throw error("Index % out of range in array dimension with length %", i, len);
	}

	// Convert indexes to positive 1-based indexes.
	void normalize_indexes(index_sequence &indexes)
	{
		// TODO: tag this as unlikely in C++20.
		if (size_type(indexes.size()) != ndim()) {
			throw error("Trying to index % dimensions in array with % dimensions", indexes.size(), ndim());
		}

		for (size_type k = 0; k < ndim(); k++)
		{
			auto i = indexes[k];
			auto dim = shape(k);

			if (i >= -dim && i < 0)
			{
				indexes[k] = dim + i;
			}
			else if (!(i > 0 && i <= dim))
			{
				throw error("Index % out of range in array dimension % with size %", i, k+1, dim);
			}
		}
	}

	void alloc_dims(size_type n)
	{
		m_dim.dx.shape = reinterpret_cast<size_type*>(utils::alloc(sizeof(size_type) * n));
		m_dim.dx.jumps = reinterpret_cast<size_type*>(utils::alloc(sizeof(size_type) * n));
	}

    void zero()
    {
        m_ndim = 0;
        m_size = 0;
        m_data = nullptr;
        m_dim.dx.jumps = nullptr;
        m_dim.dx.shape = nullptr;
    }

	// Returns true if a reallocation took place, false otherwise.
	bool expand(intptr_t requested)
	{
		assert(ndim() == 1);

		if (requested > capacity())
		{
			auto previous_capacity = (std::max<size_type>)(this->capacity(), 8);
			auto capacity = utils::find_capacity(requested, previous_capacity);
			m_dim.d1.capacity = capacity;

			if (m_data) {
				m_data = utils::reallocate<value_type>(m_data, size(), capacity);
			}
			else {
				m_data = utils::allocate<value_type>(capacity);
			}

			return true;
		}

		return false;
	}

	void init_shape(const index_sequence &indexes)
	{
		std::copy(indexes.begin(), indexes.end(), m_dim.dx.shape);
	}

	void init_jumps()
	{
		set_jump(0, 1);

		for (intptr_t i = 1; i < ndim(); ++i)
		{
			auto jmp = this->shape(i-1) * this->jump(i-1);
			set_jump(i, jmp);
		}
	}

	// 0-based index.
	intptr_t shape(intptr_t i) const
	{
		return m_dim.dx.shape[i];
	}

	void set_shape(intptr_t i, intptr_t value)
	{
		m_dim.dx.shape[i] = value;
	}

	// 0-based index.
	intptr_t jump(intptr_t i) const
	{
		return m_dim.dx.jumps[i];
	}

	void set_jump(intptr_t i, intptr_t value)
	{
		m_dim.dx.jumps[i] = value;
	}

	void alloc_data_ndim(size_type count, const value_type &default_value = value_type())
	{
		m_data = utils::allocate<value_type>(count);

		// Scalar types are already 0-initialized by calloc(), so we don't need to do anything for them.
		// Complex are handled separately.
		if constexpr (traits::is_complex<T>::value)
		{
			if (default_value != 0.0) {
				copy_construct_n(begin(), count, default_value);
			}
		}
		else if (!has_scalar_type || default_value) {
			copy_construct_n(begin(), count, default_value);
		}
	}

	void copy_construct(iterator first, iterator last, const value_type &value)
    {
	    for (auto it = first; it != last; it++)
        {
	        new(it) value_type(value);
        }
    }

    void copy_construct_n(iterator first, size_type count, const value_type &value)
    {
	    copy_construct(first, first + count, value);
    }

    intptr_t d2_to_base0(intptr_t row, intptr_t col) const
	{
		assert(ndim() <= 2);
		assert(row > 0 && col > 0);

		return (col - 1) * nrow() + row - 1;
	}

	// Number of dimensions in the array.
	size_type m_ndim;

	// Number of elements in the array.
	size_type m_size;

	// Dimensionality of the array.
	union {
		// 1-dimension array.
		struct {
			size_type capacity;  // size of the data.
		} d1;

		// 2-dimension array.
		struct {
			size_type nrow;      // number of rows.
			size_type ncol;      // number of columns.

		} d2;

		// N-dimension array.
		struct {
			size_type *shape;    // dimensions of the array.
			size_type *jumps;    // offsets to jump along dimensions.
		} dx;
	} m_dim;

	// Data of the array.
	value_type *m_data;
};

} // phonometrica

#endif // PHONOMETRICA_ARRAY_HPP
