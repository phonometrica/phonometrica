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
 * Created: 12/07/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: A polymorphic container for any value, similar to C++17 std::any. Primitive Phonometrica types             *
 * (corresponding to null, bool, Integer, Float, String) are stored unboxed, along with their type. Non-primitive      *
 * types (e.g. Table) are implemented as a managed pointer to a boxed object (see Handle<T>).                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_VARIANT_HPP
#define PHONOMETRICA_VARIANT_HPP

#include <phon/string.hpp>
#include <phon/runtime/meta.hpp>
#include <phon/runtime/typed_object.hpp>

namespace phonometrica {


class Class;
struct Alias;


class Variant final
{
	enum class Datatype
	{
		Null    = 0,
		Boolean = 1 << 0,
		Integer = 1 << 1,
		Float   = 1 << 2,
		String  = 1 << 3,
		Object  = 1 << 4,
		Alias   = 1 << 5
	};

	static constexpr int number_mask = static_cast<unsigned>(Datatype::Integer) | static_cast<unsigned>(Datatype::Float);

public:

	Variant();

	Variant(const Variant &other);

	Variant(Variant &other);

	Variant(Variant &&other) noexcept;

	Variant(std::nullptr_t) : Variant() { }

	Variant(bool val);

	Variant(intptr_t val);

	Variant(double val);

	Variant(const char *str) : Variant(String(str)) {}

	Variant(Substring str) : Variant(String(str)) { }

	Variant(Object *obj);

	Variant(String s);

#ifdef PHON_WITH_QT
	Variant(QString s) : Variant(String(s)) { }
#endif

	template<class T>
	Variant(T &&val) :
			m_data_type(Datatype::Object)
	{
		using Type = typename std::remove_reference<typename std::remove_cv<T>::type>::type;
		as.object = new TObject<Type>(std::forward<Type>(val));
	}

	// Note: beware of C++'s "most vexing parse" when using this constructor. The following code:
	// 		Variant v(Handle<List>(lst));
	// won't work because it will be interpreted as a function declaration. Instead, use one of
	// the following declarations:
	//		Variant v{Handle<List>(lst)};   // brace-initializer
	//		Variant v((Handle<List>(lst))); // extra pair of parentheses
	// or better still, use the constructor which takes a universal reference and wraps the value
	// automatically, like so:
	//		Variant v(lst);
	template<class T>
	Variant(Handle<T> val) :
			m_data_type(Datatype::Object)
	{
		new(&as.storage) Handle<T>(std::move(val));
	}

	~Variant();

	void swap(Variant &other) noexcept;

	Variant &operator=(Variant other);

	Datatype data_type() const { return m_data_type; }

	Class *get_class() const;

	bool empty() const;

	void clear();

	bool is_object() const;

	bool is_integer() const { return m_data_type == Datatype::Integer; }

	bool is_float() const { return m_data_type == Datatype::Float; }

	bool is_number() const { return static_cast<unsigned >(m_data_type) & number_mask; }

	bool is_alias() const { return m_data_type == Datatype::Alias; }

	bool is_null() const { return m_data_type == Datatype::Null; }

	bool is_string() const { return m_data_type == Datatype::String; }

	const std::type_info *type_info() const;

	Variant & make_alias();

	void unalias();

	Variant &resolve();

	const Variant &resolve() const;

	String class_name() const;

	void traverse(const GCCallback &callback);

	bool operator==(const Variant &other) const;

	bool operator!=(const Variant &other) const;

	bool operator<(const Variant &other) const;

	int compare(const Variant &other) const;

	String to_string(bool quote = false) const;

	String to_json(int spacing = 1) const;

	bool to_boolean() const;

	intptr_t to_integer() const;

	double to_float() const;

	size_t hash() const;

	double get_number() const;

	template<class T>
	Handle<T> handle() { return reinterpret_cast<Handle<T> &>(as.storage); }

	Variant & unshare();

private:

	friend class Runtime;

	template<class T>
	friend T &cast(Variant &var);

	template<class T>
	friend T &raw_cast(Variant &var);

	template<class T>
	friend bool check_type(const Variant &var);

	void retain();

	void release();

	void zero();

	void finalize(); // for the runtime only

	void copy_fields(const Variant &other);

	String as_string() const;

	String to_json(const String &s) const;

	using largest_type_t = typename std::conditional<sizeof(void *) >= sizeof(double), void *, double>::type;
	using storage_t = std::aligned_storage<sizeof(largest_type_t), alignof(largest_type_t)>::type;

	// Boxed or unboxed value.
	union Storage
	{
		storage_t storage;
		Object *object;
		Alias *alias;
	} as;

	// Type of the storage.
	Datatype m_data_type;
};


//---------------------------------------------------------------------------------------------------------------------

template<class T>
bool check_type(const Variant &v)
{
	auto &var = v.resolve();
	using Type = typename traits::bare_type<T>::type;
	return var.is_object() && var.type_info() == &typeid(Type);
}

template<>
inline bool check_type<bool>(const Variant &var)
{
	return var.resolve().data_type() == Variant::Datatype::Boolean;
}

template<>
inline bool check_type<intptr_t>(const Variant &var)
{
	return var.resolve().data_type() == Variant::Datatype::Integer;
}

template<>
inline bool check_type<double>(const Variant &var)
{
	return var.resolve().data_type() == Variant::Datatype::Float;
}

template<>
inline bool check_type<String>(const Variant &var)
{
	return var.resolve().data_type() == Variant::Datatype::String;
}

//------------------------------------------------------------------------------------------------------------------

template<class T>
T &raw_cast(Variant &var)
{
	return reinterpret_cast<Handle<T> &>(var.resolve().as.storage).value();
}


template<class T>
const T &raw_cast(const Variant &var)
{
	return raw_cast<T>(const_cast<Variant &>(var));
}

template<>
inline bool &raw_cast<bool>(Variant &var)
{
	return reinterpret_cast<bool &>(var.resolve().as.storage);
}

template<>
inline intptr_t &raw_cast<intptr_t>(Variant &var)
{
	return reinterpret_cast<intptr_t &>(var.resolve().as.storage);
}

template<>
inline double &raw_cast<double>(Variant &var)
{
	return reinterpret_cast<double &>(var.resolve().as.storage);
}

template<>
inline String &raw_cast<String>(Variant &var)
{
	return reinterpret_cast<String &>(var.resolve().as.storage);
}

//------------------------------------------------------------------------------------------------------------------

template<class T>
T &cast(Variant &v)
{
	auto &var = v.resolve();
	using Type = typename traits::bare_type<T>::type;
	if (likely(var.data_type() == Variant::Datatype::Object))
	{
		auto ptr = reinterpret_cast<TObject<Type> *>(var.as.object);

		if (likely(ptr->type_info() == &typeid(Type))) {
			return ptr->value();
		}
	}

	throw error("[Cast error] Expected a %, got a %", meta::get_class_name<Type>(), var.class_name());
}


template<class T>
const T &cast(const Variant &var)
{
	return cast<T>(const_cast<Variant &>(var));
}

template<>
inline bool &cast<bool>(Variant &v)
{
	auto &var = v.resolve();

	if (likely(check_type<bool>(var))) {
		return raw_cast<bool>(var);
	}

	throw error("[Cast error] Expected a %, got a %", meta::get_class_name<bool>(), var.class_name());
}

template<>
inline intptr_t &cast<intptr_t>(Variant &v)
{
	auto &var = v.resolve();

	if (likely(check_type<intptr_t>(var))) {
		return raw_cast<intptr_t>(var);
	}

	throw error("[Cast error] Expected a %, got a %", meta::get_class_name<intptr_t>(), var.class_name());
}

template<>
inline double &cast<double>(Variant &v)
{
	auto &var = v.resolve();

	if (likely(check_type<double>(var))) {
		return raw_cast<double>(var);
	}

	throw error("[Cast error] Expected a %, got a %", meta::get_class_name<double>(), var.class_name());
}

template<>
inline String &cast<String>(Variant &v)
{
	auto &var = v.resolve();

	if (likely(check_type<String>(var))) {
		return raw_cast<String>(var);
	}

	throw error("[Cast error] Expected a %, got a %", meta::get_class_name<String>(), var.class_name());
}

//------------------------------------------------------------------------------------------------------------------

// Aliases are used to implement references. A value that is shared by several references is stored in an alias instead
// of being stored in a variant directly.
struct Alias final
{
	explicit Alias(Variant v) : ref_count(1), variant(std::move(v)) { }

	~Alias() = default;

	void retain() { ++ref_count; }

	void release()
	{
		if (--ref_count == 0) {
			delete this;
		}
	}

	int32_t ref_count;
	Variant variant;
};


inline
std::ostream &operator<<(std::ostream &stream, const Variant &v)
{
	auto s = v.to_string();
	stream.write(s.data(), s.size());

	return stream;
}

} // namespace phonometrica


//----------------------------------------------------------------------------------------------------------------------

namespace std {

template<>
struct hash<phonometrica::Variant>
{
	size_t operator()(const phonometrica::Variant &v) const
	{
		return v.hash();
	}
};

} // namespace std

#endif // PHONOMETRICA_VARIANT_HPP
