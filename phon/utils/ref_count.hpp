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
 * Purpose: intrusive reference count.                                                                                *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_REF_COUNT_HPP
#define PHONOMETRICA_REF_COUNT_HPP

#include <atomic>
#include <utility>
#include <phon/runtime/definitions.hpp>

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
class IntrusivePtr
{
public:

	// Dummy struct to construct a handle from a raw pointer without retaining it.
	struct Raw { };

	IntrusivePtr()
	{ ptr = nullptr; }

	template<typename... Args>
	IntrusivePtr(Args... args)
	{
		ptr = new T(std::forward<Args>(args)...);
	}

	template<class U>
	IntrusivePtr(IntrusivePtr<U> h)
	{
		static_assert(std::is_base_of<T, U>::value, "U must be a subclass of T");
		ptr = h.abandon();
	}

	explicit IntrusivePtr(T *value) {
		ptr = value;
		retain();
	}

	IntrusivePtr(T *value, Raw) {
		ptr = value;
	}

	IntrusivePtr(const IntrusivePtr &other) {
		ptr = other.get();
		retain();
	}

	IntrusivePtr(IntrusivePtr &&other) noexcept {
		ptr = other.get();
		other.zero();
	}

	~IntrusivePtr() noexcept {
		release();
	}

	IntrusivePtr &operator=(const IntrusivePtr &other) noexcept
	{
		if (this != &other)
		{
			release();
			ptr = other.ptr;
			retain();
		}

		return *this;
	}

	IntrusivePtr &operator=(IntrusivePtr &&other) noexcept
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

	bool operator==(const IntrusivePtr &other) const {
		return ptr == other.get();
	}

	bool operator!=(const IntrusivePtr &other) const {
		return ptr != other.get();
	}

	void swap(IntrusivePtr &other) noexcept {
		std::swap(ptr, other.ptr);
	}

	T *drop()
	{
		auto tmp = ptr;
		ptr = nullptr;
		return tmp;
	}

private:

	template<typename> friend class IntrusivePtr;

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

#endif // PHONOMETRICA_REF_COUNT_HPP
