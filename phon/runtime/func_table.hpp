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
 * Created: 31/05/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: Table builtin functions.                                                                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FUNC_TABLE_HPP
#define PHONOMETRICA_FUNC_TABLE_HPP

#include <phon/runtime/table.hpp>
#include <phon/runtime.hpp>

namespace phonometrica {

static Variant table_init(Runtime &rt, std::span<Variant>)
{
	return make_handle<Table>(&rt);
}

static Variant table_get_item(Runtime &, std::span<Variant> args)
{
	auto &tab = cast<Table>(args[0]);
	return tab.get(args[1]);
}

static Variant table_get_field(Runtime &rt, std::span<Variant> args)
{
	auto &tab = cast<Table>(args[0]);
	auto &key = cast<String>(args[1]);

	if (key == rt.length_string) {
		return tab.size();
	}
	else if (key == "keys") {
		return make_handle<List>(&rt, tab.keys());
	}
	else if (key == "values") {
		return make_handle<List>(&rt, tab.values());
	}

	throw error("[Index error] Table type has no member named \"%\"", key);
}

static Variant table_set_item(Runtime &, std::span<Variant> args)
{
	auto &map = cast<Table>(args[0]).map();
	map[args[1].resolve()] = std::move(args[2].resolve());

	return Variant();
}

static Variant table_contains(Runtime &, std::span<Variant> args)
{
	auto &map = cast<Table>(args[0]).map();
	return map.contains(args[1]);
}

static Variant table_is_empty(Runtime &, std::span<Variant> args)
{
	auto &map = cast<Table>(args[0]).map();
	return map.empty();
}

static Variant table_clear(Runtime &, std::span<Variant> args)
{
	auto &map = cast<Table>(args[0]).map();
	map.clear();

	return Variant();
}

static Variant table_remove(Runtime &, std::span<Variant> args)
{
	auto &map = cast<Table>(args[0]).map();
	map.erase(args[1].resolve());

	return Variant();
}

static Variant table_get1(Runtime &, std::span<Variant> args)
{
	auto &map = cast<Table>(args[0]).map();
	auto it = map.find(args[1].resolve());

	return (it != map.end()) ? it->second : Variant();
}

static Variant table_get2(Runtime &, std::span<Variant> args)
{
	auto &map = cast<Table>(args[0]).map();
	auto it = map.find(args[1].resolve());

	return (it != map.end()) ? it->second : args[2];
}

} // namespace phonometrica

#endif // PHONOMETRICA_FUNC_TABLE_HPP
