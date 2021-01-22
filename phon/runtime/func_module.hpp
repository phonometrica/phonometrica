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
 * Created: 31/05/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: Module builtin functions.                                                                                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONSCRIPT_FUNC_MODULE_HPP
#define PHONSCRIPT_FUNC_MODULE_HPP

#include <phon/runtime/module.hpp>

namespace phonometrica {

static Variant module_init(Runtime &rt, std::span<Variant>args)
{
	auto &name = cast<String>(args[0]);
	return make_handle<Module>(&rt, name);
}

static Variant module_get_attr(Runtime &, std::span<Variant> args)
{
	auto &m = cast<Module>(args[0]);
	auto key = cast<String>(args[1]);

	return m.get(key);
}

static Variant module_set_attr(Runtime &, std::span<Variant> args)
{
	auto &m = cast<Module>(args[0]);
	auto key = cast<String>(args[1]);
	m[key] = args[2].resolve();

	return Variant();
}

static Variant module_contains(Runtime &, std::span<Variant>args)
{
	auto &mod = cast<Module>(args[0]);
	auto &key = cast<String>(args[1]);

	return mod.contains(key);
}

} // namespace phonometrica

#endif // PHONSCRIPT_FUNC_MODULE_HPP
