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
 * Purpose: intrusive reference count.                                                                                 *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_REFCOUNT_HPP
#define PHONOMETRICA_REFCOUNT_HPP

#include <atomic>
#include <utility>
#include <phon/definitions.hpp>

namespace phonometrica {

// This class provides reference counting functionality for all managed heap-allocated structures. Subclasses should
// generally be wrapped in a Handle<T>, which has value semantics and manages reference counts automatically.

template<typename T, typename SizeType = std::size_t>
class Countable
{
public:

	typedef size_t size_type;

	void retain() noexcept {
		add_reference();
	}

	void release() noexcept
	{
		if (remove_reference()) {
			delete static_cast<T*>(this);
		}
	}

	void add_reference() noexcept {
		ref_count.fetch_add(1, std::memory_order_relaxed);
	}

	bool remove_reference() noexcept {
		return ref_count.fetch_sub(1, std::memory_order_relaxed) == 1;
	}

	bool shared() const noexcept {
		return ref_count.load(std::memory_order_relaxed) > 1;
	}

	bool unique() const noexcept {
		return ref_count.load(std::memory_order_relaxed) == 1;
	}

	size_type use_count() const noexcept {
		return ref_count.load(std::memory_order_relaxed);
	}

protected:

	Countable() noexcept = default;

	std::atomic<size_type> ref_count { 1 };
};


//----------------------------------------------------------------------------------------------------------------------

// A ligth-weight replacement for std::shared_ptr. This is basically a smart pointer for subclasses of Countable,
// which manages reference counting automatically.

template<typename T>
class Handle
{
public:

	// Dummy struct to construct a handle from a raw pointer without retaining it.
	struct Retain { };

	Handle()
	{ ptr = nullptr; }

	template<typename... Args>
	Handle(Args... args)
	{
		ptr = new T(std::forward<Args>(args)...);
	}

	template<class U>
	Handle(Handle<U> h)
	{
		static_assert(std::is_base_of<T, U>::value, "U must be a subclass of T");
		ptr = h.abandon();
	}

	// When constructing from a raw pointer, the object is initialized with a count of 1 so we don't retain by default...
	explicit Handle(T *value) {
		ptr = value;
	}

	// ... but we can explicitly retain if needed.
	Handle(T *value, Retain) {
		ptr = value;
		retain();
	}

	Handle(const Handle &other) {
		ptr = other.get();
		retain();
	}

	Handle(Handle &&other) noexcept {
		ptr = other.get();
		other.zero();
	}

	~Handle() noexcept {
		release();
	}

	Handle &operator=(const Handle &other) noexcept
	{
		if (this != &other)
		{
			release();
			ptr = other.ptr;
			retain();
		}

		return *this;
	}

	Handle &operator=(Handle &&other) noexcept
	{
		std::swap(ptr, other.ptr);
		return *this;
	}

//	template<typename U>
//	Handle<T> &operator=(Handle<U> other) noexcept
//	{
//		static_assert(std::is_base_of<T,U>::value, "T must be a base of U");
//
//		Release();
//		ptr = other.get();
//		other.zero();
//
//		return *this;
//	}

	T* get() const {
		return ptr;
	}

	T& operator*() const {
		return *ptr;
	}

	T* operator->() const {
		return ptr;
	}

	operator T*() const {
		return ptr;
	}

	operator bool() const {
		return ptr != nullptr;
	}

	bool operator==(const Handle &other) const {
		return ptr == other.get();
	}

	bool operator!=(const Handle &other) const {
		return ptr != other.get();
	}

	void swap(Handle &other) noexcept {
		std::swap(ptr, other.ptr);
	}

	T *drop()
	{
		auto tmp = ptr;
		ptr = nullptr;
		return tmp;
	}

private:

	template<typename> friend class Handle;

	void zero() noexcept {
		ptr = nullptr;
	}

	void retain() noexcept {
		if (ptr) ptr->retain();
	}

	void release() noexcept {
		if (ptr) ptr->release();
	}

	T *ptr;

};


} // namespace phonometrica

#endif // PHONOMETRICA_REFCOUNT_HPP
