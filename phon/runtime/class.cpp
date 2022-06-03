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
 * Created: 22/05/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime/class.hpp>
#include <phon/runtime/function.hpp>

namespace phonometrica {

String Class::init_string("init");


Class::Class(String name, Class *parent, const std::type_info *info, Index index) :
	_name(std::move(name)), _info(info), _bases(parent ? parent->_bases : std::vector<Class*>()), index(index)
{
	_depth = _bases.size();
	_bases.push_back(this);
}

bool Class::inherits(const Class *base) const
{
	return _bases[base->depth()] == base && base->depth() <= this->depth();
}

int Class::get_distance(const Class *base) const
{
	return _bases[base->depth()] == base ? int(this->depth() - base->depth()) : -1;
}

Handle<Function> Class::get_method(const String &name)
{
	auto it = members.find(name);
	if (it == members.end()) {
		throw error("[Index error] Class % does not have a method called \"%\"", this->name(), name);
	}

	return it->second.handle<Function>();
}

Handle<Function> Class::get_constructor()
{
	return get_method(init_string);
}

void Class::add_initializer(NativeCallback cb, std::initializer_list<Handle<Class>> sig, ParamBitset ref)
{
	add_method(init_string, std::move(cb), sig, ref);
}

void Class::add_method(const String &name, NativeCallback cb, std::initializer_list<Handle<Class>> sig, ParamBitset ref)
{
	auto it = members.find(name);
	// This is fine as long as we have a single runtime.
	auto rt = static_cast<Collectable*>(object())->runtime;
	if (it == members.end())
	{
		members.insert({ name, make_handle<Function>(rt, rt, name, std::move(cb), sig, ref) });
	}
	else
	{
		auto ctor = it->second.handle<Function>();
		ctor->add_closure(make_handle<Closure>(rt, std::make_shared<NativeRoutine>(name, std::move(cb), sig, ref)));
	}

}

void Class::add_method(const String &name, Handle<Function> f)
{
	members[name] = std::move(f);
}

void Class::add_initializer(Handle<Function> f)
{
	add_method(init_string, std::move(f));
}

void Class::finalize()
{
	for (auto &pair : members) {
		pair.second.clear();
	}
}

void Class::traverse_members(const GCCallback &callback)
{
	for (auto &pair : members) {
		pair.second.traverse(callback);
	}
}

} // namespace phonometrica
