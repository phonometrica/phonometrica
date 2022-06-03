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
 * Created: 04/06/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime/class.hpp>
#include <phon/runtime/module.hpp>
#include <phon/runtime/function.hpp>

namespace phonometrica {

Variant &Module::get(const String &key)
{
	auto it = members.find(key);

	if (it == members.end()) {
		throw error("[Index error] Missing key in module \"%\": \"%\"", _name, key);
	}

	return it->second;
}

bool Module::contains(const String &key) const
{
	return members.contains(key);
}

void Module::define(const String &name, Variant value)
{
	members[name] = std::move(value);
}

void Module::traverse(const GCCallback &callback)
{
	for (auto &pair : members) {
		pair.second.traverse(callback);
	}
}

void Module::define(Runtime *rt, const String &name, NativeCallback cb, std::initializer_list<Handle<Class>> sig, ParamBitset ref)
{
	members[name] = make_handle<Function>(rt, rt, name, std::move(cb), sig, ref);
}

} // namespace phonometrica