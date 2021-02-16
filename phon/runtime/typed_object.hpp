/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
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
 * Created: 22/05/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: template that wraps a value in an object. In general, this should not be used directly. Use                *
 * Runtime::create<T>() instead.                                                                                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TYPED_OBJECT_HPP
#define PHONOMETRICA_TYPED_OBJECT_HPP

#include <phon/runtime/object.hpp>
#include <phon/runtime/traits.hpp>
#include <phon/runtime/class_descriptor.hpp>

namespace phonometrica {

namespace detail {

// Select base class at compile time depending on whether the type is collectable or not.
template<typename T>
using object_base = typename std::conditional<traits::is_collectable<T>::value, Collectable, Atomic>::type;

} // namespace detail


//----------------------------------------------------------------------------------------------------------------------

// All non-primitive types exposed to Phonometrica are wrapped in a typed TObject.

template<class T>
class TObject final : public detail::object_base<T>
{
public:

	using base_type = detail::object_base<T>;

	// Constructor for non collectable objects
	template<typename ...Params>
	explicit TObject(Params &&... params) :
			base_type(detail::ClassDescriptor<T>::get()), m_value(std::forward<Params>(params)...)
	{ }

	// Constructor for collectable objects
	template<typename ...Params>
	explicit TObject(Runtime *rt, Params &&... params) :
			base_type(detail::ClassDescriptor<T>::get(), rt), m_value(std::forward<Params>(params)...)
	{ }

	~TObject() = default;

	T &value()
	{ return m_value; }

	const T &value() const
	{ return m_value; }


private:

	T m_value;
};


//---------------------------------------------------------------------------------------------------------------------

// Smart pointer for TObject.
template<typename T>
class Handle
{
public:

	using object_type = std::conditional<traits::is_object<T>::value, T, TObject<T>>;

	Handle()
	{ ptr = nullptr; }

	// By default we retain the value.
	explicit Handle(object_type *value) {
		ptr = value;
		retain();
	}

	// ... but we can simply wrap the pointer without retaining it if needed.
	Handle(object_type *value, std::false_type) {
		ptr = value;
	}

	Handle(const Handle &other) {
		ptr = other.ptr;
		retain();
	}

	Handle(Handle &&other) noexcept {
		ptr = other.ptr;
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

	T* get() const {
		return &ptr->value();
	}

	T& operator*() const {
		return ptr->value();
	}

	T* operator->() const {
		return &ptr->value();
	}

	operator bool() const {
		return ptr != nullptr;
	}

	bool operator==(const Handle &other) const {
		return ptr == other.ptr;
	}

	bool operator!=(const Handle &other) const {
		return ptr != other.ptr;
	}

	void swap(Handle &other) noexcept {
		std::swap(ptr, other.ptr);
	}

	object_type *drop()
	{
		auto tmp = ptr;
		this->zero();
		return tmp;
	}

	void zero() noexcept {
		ptr = nullptr;
	}

	object_type *object() {
		return ptr;
	}

	const object_type *object() const {
		return ptr;
	}

	T &value() {
		return ptr->value();
	}

	const T &value() const {
		return ptr->value();
	}

private:

	template<typename> friend class Handle;

	void retain() noexcept {
		if (ptr) ptr->retain();
	}

	void release() noexcept {
		if (ptr) ptr->release();
	}

	object_type *ptr;
};


//---------------------------------------------------------------------------------------------------------------------

template<class T, class... Args>
Handle<T> make_handle(Args... args)
{
	if constexpr (traits::is_object<T>::value) {
		return Handle<T>(new T(std::forward<Args>(args)...), std::false_type());
	}
	else {
		return Handle<T>(new TObject<T>(std::forward<Args>(args)...), std::false_type());
	}
}


} // namespace phonometrica

#endif // PHONOMETRICA_TYPED_OBJECT_HPP
