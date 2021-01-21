/***********************************************************************************************************************
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
 * Created: 04/06/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: a module provides a namespace for Phonometrica code.                                                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_MODULE_HPP
#define PHONOMETRICA_MODULE_HPP

#include <phon/runtime/variant.hpp>
#include <phon/dictionary.hpp>

namespace phonometrica {

class Module final
{
public:

	using Storage = Dictionary<Variant>;
	using iterator = Storage::iterator;
	using value_type = Storage::value_type;

	explicit Module(const String &name) : _name(name) { }

	Module(const Module &) = delete;


	String name() const { return _name; }

	Variant &operator[](const String &key) { return members[key]; }

	iterator find(const String &key) { return members.find(key); }

	iterator begin() { return members.begin(); }

	iterator end() { return members.end(); }

	void insert(value_type v) { members.insert(std::move(v)); }

	Variant &get(const String &key);

	bool contains(const String &key) const;

	void define(const String &name, Variant value);

	void define(Runtime *rt, const String &name, NativeCallback cb, std::initializer_list<Handle<Class>> sig, ParamBitset ref = ParamBitset());

	void traverse(const GCCallback &callback);

private:

	friend class Runtime;

	String _name;

	Storage members;
};

namespace meta {

static inline
void traverse(Module &value, const GCCallback &callback)
{
	value.traverse(callback);
}
} // meta

} // namespace phonometrica

#endif // PHONOMETRICA_MODULE_HPP
