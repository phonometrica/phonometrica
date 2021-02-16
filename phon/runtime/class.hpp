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
 * Purpose: Class object. Each object stores a pointer to its class, which provides runtime type information (RTTI)    *
 * and basic polymorphic methods.                                                                                      *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CLASS_HPP
#define PHONOMETRICA_CLASS_HPP

#include <typeinfo>
#include <vector>
#include <phon/string.hpp>
#include <phon/runtime/typed_object.hpp>
#include <phon/dictionary.hpp>
#include <phon/runtime/callback.hpp>

namespace phonometrica {

class Function;

// Classes are objects too.
class Class final
{

public:

	enum class Index
	{
		Object,
		Class,
		Null,
		Boolean,
		Number,
		Integer,
		Float,
		String,
		Regex,
		List,
		Array,
		Table,
		Set,
		File,
		Function,
		Closure,
		Module,
		Iterator,
		ListIterator,
		TableIterator,
		StringIterator,
		FileIterator,
		RegexIterator,
		Foreign
	};


	Class(String name, Class *parent, const std::type_info *info, Index index = Index::Foreign);

	Class(const Class &) = delete;

	Class(Class &&) = delete;

	~Class() = default;

	String name() const { return _name; }

	size_t depth() const { return _depth; }

	bool inherits(const Class *base) const;

	int get_distance(const Class *base) const;

	const std::type_info *type_info() const { return _info; }

	template<class T>
	static Class *get()
	{
		using Type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
		return detail::ClassDescriptor<Type>::get();
	}

	template<class T>
	static String get_name()
	{
		return get<T>()->name();
	}

	bool operator==(const Class &other) const { return this == &other; }

	Object *object() { return _object; }

	Handle<Function> get_constructor();

	Handle<Function> get_method(const String &name);

	void add_initializer(NativeCallback cb, std::initializer_list<Handle<Class>> sig, ParamBitset ref = ParamBitset());

	void add_initializer(Handle<Function> f);

	void add_method(const String &name, NativeCallback cb, std::initializer_list<Handle<Class>> sig, ParamBitset ref = ParamBitset());

	void add_method(const String &name, Handle<Function> f);

	void traverse_members(const GCCallback &callback);

private:

	friend class Runtime;
	friend class Object;

	// Polymorphic methods for type erasure.
	void(*destroy)(Object*) = nullptr;
	size_t (*hash)(const Object*) = nullptr;
	void (*traverse)(Collectable*, const GCCallback&) = nullptr;
	Object *(*clone)(const Object*) = nullptr;
	String (*to_string)(const Object*) = nullptr;
	int (*compare)(const Object*, const Object*) = nullptr;
	bool (*equal)(const Object*, const Object*) = nullptr;

	void set_object(Object *o) { _object = o; }

	// We need to manually finalize members that refer to a class before classes are finalized by the runtime's destructor.
	void finalize();

	// Name given to the class when it was created
	String _name;

	// Pointer to the object the class is wrapped in.
	Object *_object = nullptr;

	// Inheritance depth (0 for Object, 1 for direct subclasses of Object, etc.).
	size_t _depth;

	// C++ type for builtin types (null for user-defined types). This is used to safely downcast
	// objects to TObject<T>. Since [object] doesn't use C++'s virtual inheritance, we can't
	// use dynamic_cast for that purpose.
	const std::type_info *_info;

	// Non-owning array of Class pointers, representing the class's inheritance hierarchy. The first element represents
	// the top-most class, and is always Object. The last element is the class itself. This allows constant-time lookup
	// using the class's inheritance depth.
	std::vector<Class*> _bases;

	Dictionary<Variant> members;

	// For debugging.
	Index index;

	static String init_string;
};

//---------------------------------------------------------------------------------------------------------------------

template<class T>
Handle<Class> get_class()
{
	return Handle<Class>(static_cast<TObject<Class>*>(detail::ClassDescriptor<T>::get()->object()));
}



namespace meta {

static inline
String to_string(const Class &klass)
{
	return String::format("<class %s>", klass.name().data());
}

static inline
void traverse(Class &cls, const GCCallback &callback)
{
	cls.traverse_members(callback);
}


} // namespace phonometrica::meta

} // namespace phonometrica

#endif // PHONOMETRICA_CLASS_HPP
