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
 * Created: 05/06/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Set builtin functions.                                                                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FUNC_SET_HPP
#define PHONOMETRICA_FUNC_SET_HPP

#include <phon/runtime/set.hpp>
#include <phon/runtime.hpp>

namespace phonometrica {

static Variant set_init(Runtime &rt, std::span<Variant>)
{
	return make_handle<Set>(&rt);
}

static Variant set_get_field(Runtime &rt, std::span<Variant> args)
{
	auto &set = cast<Set>(args[0]).items();
	auto &key = cast<String>(args[1]);

	if (key == rt.length_string) {
		return set.size();
	}

	throw error("[Index error] Set type has no member named \"%\"", key);
}

static Variant set_contains(Runtime &, std::span<Variant> args)
{
	auto &set = cast<Set>(args[0]);
	return set.contains(args[1].resolve());
}

static Variant set_insert(Runtime &, std::span<Variant> args)
{
	auto &set = cast<Set>(args[0].unshare()).items();
	set.insert(args[1].resolve());

	return Variant();
}

static Variant set_remove(Runtime &, std::span<Variant> args)
{
	auto &set = cast<Set>(args[0].unshare()).items();
	set.erase(args[1].resolve());

	return Variant();
}

static Variant set_is_empty(Runtime &, std::span<Variant> args)
{
	auto &set = cast<Set>(args[0]).items();
	return set.empty();
}

static Variant set_clear(Runtime &, std::span<Variant> args)
{
	auto &set = cast<Set>(args[0].unshare()).items();
	set.clear();

	return Variant();
}

static Variant set_intersect(Runtime &rt, std::span<Variant> args)
{
	auto &lst1 = cast<Set>(args[0]).items();
	auto &lst2 = cast<Set>(args[1]).items();
	Set::Storage result;
	std::set_intersection(lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), std::inserter(result, result.begin()));

	return make_handle<Set>(&rt, std::move(result));
}

static Variant set_unite(Runtime &rt, std::span<Variant> args)
{
	auto &lst1 = cast<Set>(args[0]).items();
	auto &lst2 = cast<Set>(args[1]).items();
	Set::Storage result;
	std::set_union(lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), std::inserter(result, result.begin()));

	return make_handle<Set>(&rt, std::move(result));
}

static Variant set_subtract(Runtime &rt, std::span<Variant> args)
{
	auto &lst1 = cast<Set>(args[0]).items();
	auto &lst2 = cast<Set>(args[1]).items();
	Set::Storage result;
	std::set_difference(lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), std::inserter(result, result.begin()));

	return make_handle<Set>(&rt, std::move(result));
}

} // namespace phonometrica

#endif // PHONOMETRICA_FUNC_SET_HPP
