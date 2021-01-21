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
 * Purpose: List builtin functions.                                                                                    *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FUNC_LIST_HPP
#define PHONOMETRICA_FUNC_LIST_HPP

#include <algorithm>
#include <random>
#include <phon/runtime.hpp>

namespace phonometrica {

static Variant list_init(Runtime &rt, std::span<Variant>)
{
	return make_handle<List>(&rt);
}

static Variant list_get_item(Runtime &rt, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	if (args.size() > 2) {
		throw error("[Index error] List does not support multidimensional indexing");
	}
	if (!check_type<intptr_t>(args[1])) {
		throw error("[Index error] List index must be an Integer, not a %", args[1].class_name());
	}
	auto i = cast<intptr_t>(args[1]);

	return rt.needs_reference() ? lst.at(i).make_alias() : lst.at(i).resolve();
}

static Variant list_get_field(Runtime &rt, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	auto &key = cast<String>(args[1]);

	if (key == rt.length_string) {
		return lst.size();
	}
	else if (key == "first") {
		return lst.at(1);
	}
	else if (key == "last") {
		return lst.at(-1);
	}

	throw error("[Index error] List type has no member named \"%\"", key);
}

static Variant list_set_item(Runtime &, std::span<Variant> args)
{
	// TODO: no unshare in set_item!!!
	auto &lst = cast<List>(args[0]).items();
	intptr_t i = cast<intptr_t>(args[1]);
	lst.at(i) = std::move(args[2].resolve());

	return Variant();
}

static Variant list_contains(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	return lst.contains(args[1]);
}

static Variant list_first(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	if (lst.empty()) {
		throw error("[Index error] Cannot get first element in empty list");
	}
	return lst.first();
}

static Variant list_find1(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	return lst.find(args[1].resolve());
}

static Variant list_find2(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	intptr_t i = cast<intptr_t>(args[1]);
	return lst.find(args[1].resolve(), i);
}

static Variant list_rfind_back1(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	return lst.rfind(args[1]);
}

static Variant list_rfind_back2(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	intptr_t i = cast<intptr_t>(args[1]);
	return lst.rfind(args[1], i);
}

static Variant list_last(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	if (lst.empty()) {
		throw error("[Index error] Cannot get last element in empty list");
	}
	return lst.last();
}

static Variant list_left(Runtime &rt, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	intptr_t count = cast<intptr_t>(args[1]);
	Array<Variant> result;

	for (intptr_t i = 1; i <= count; i++) {
		result.append(lst.at(i));
	}

	return make_handle<List>(&rt, std::move(result));
}

static Variant list_right(Runtime &rt, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	intptr_t count = cast<intptr_t>(args[1]);
	Array<Variant> result;
	intptr_t limit = lst.size() - count;

	for (intptr_t i = lst.size(); i > limit; i--) {
		result.append(lst.at(i));
	}

	return make_handle<List>(&rt, std::move(result));
}

static Variant list_join(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	auto &delim = cast<String>(args[1]);
	String result;

	for (intptr_t i = 1; i <= lst.size(); i++)
	{
		result.append(lst[i].to_string());
		if (i != lst.size()) {
			result.append(delim);
		}
	}

	return result;
}

static Variant list_clear(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	lst.clear();

	return Variant();
}

static Variant list_append(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	lst.append(args[1].resolve());

	return Variant();
}

//static Variant list_append2(Runtime &, std::span<Variant> args)
//{
//	auto &lst1 = cast<List>(args[0]).items();
//	auto &lst2 = cast<List>(args[0]).items();
//
//	for (auto &item : lst2) {
//		lst1.append(item.resolve());
//	}
//
//	return Variant();
//}

static Variant list_prepend(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	lst.prepend(args[1].resolve());

	return Variant();
}

//static Variant list_prepend2(Runtime &, std::span<Variant> args)
//{
//	auto &lst1 = cast<List>(args[0]).items();
//	auto &lst2 = cast<List>(args[0]).items();
//
//	for (intptr_t i = lst2.size(); i > 0; i--) {
//		lst1.prepend(lst2[i].resolve());
//	}
//
//	return Variant();
//}

static Variant list_is_empty(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();

	return lst.empty();
}

static Variant list_pop(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();

	return lst.take_last().resolve();
}

static Variant list_shift(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();

	return lst.take_first().resolve();
}

static Variant list_sort(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	std::sort(lst.begin(), lst.end());

	return Variant();
}

static Variant list_is_sorted(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	return std::is_sorted(lst.begin(), lst.end());
}

static Variant list_reverse(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	std::reverse(lst.begin(), lst.end());

	return Variant();
}

static Variant list_remove(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	lst.remove(args[1].resolve());

	return Variant();
}

static Variant list_remove_first(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	lst.remove_first(args[1].resolve());

	return Variant();
}

static Variant list_remove_last(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	lst.remove_last(args[1].resolve());

	return Variant();
}

static Variant list_remove_at(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	intptr_t pos = cast<intptr_t>(args[1]);
	lst.remove_at(pos);

	return Variant();
}

static Variant list_shuffle(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(lst.begin(), lst.end(), g);

	return Variant();
}

static Variant list_sample(Runtime &rt, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	intptr_t n = cast<intptr_t>(args[1]);
	Array<Variant> result;
	std::random_device rd;
	std::mt19937 g(rd());
	std::sample(lst.begin(), lst.end(), std::back_inserter(result), n, g);

	return make_handle<List>(&rt, std::move(result));
}

static Variant list_insert(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	intptr_t pos = cast<intptr_t>(args[1]);
	lst.insert(pos, args[2].resolve());

	return Variant();
}

static Variant list_intersect(Runtime &rt, std::span<Variant> args)
{
	auto &lst1 = cast<List>(args[0]).items();
	auto &lst2 = cast<List>(args[1]).items();
	List::Storage result;
	std::set_intersection(lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), std::back_inserter(result));

	return make_handle<List>(&rt, std::move(result));
}

static Variant list_unite(Runtime &rt, std::span<Variant> args)
{
	auto &lst1 = cast<List>(args[0]).items();
	auto &lst2 = cast<List>(args[1]).items();
	List::Storage result;
	std::set_union(lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), std::back_inserter(result));

	return make_handle<List>(&rt, std::move(result));
}

static Variant list_subtract(Runtime &rt, std::span<Variant> args)
{
	auto &lst1 = cast<List>(args[0]).items();
	auto &lst2 = cast<List>(args[1]).items();
	List::Storage result;
	std::set_difference(lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), std::back_inserter(result));

	return make_handle<List>(&rt, std::move(result));
}

static Variant list_sorted_find(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0]).items();
	auto it = std::lower_bound(lst.begin(), lst.end(), args[1].resolve());
	intptr_t pos = 0; // not found
	if (it != lst.end()) pos = (it - lst.begin()) + 1;

	return pos;
}

static Variant list_sorted_insert(Runtime &, std::span<Variant> args)
{
	auto &lst = cast<List>(args[0].unshare()).items();
	auto &value = args[1].resolve();
	auto it = std::lower_bound(lst.begin(), lst.end(), value);
	if (it == lst.end() || value < *it)
		lst.insert(it, value);

	return Variant();
}

} // namespace phonometrica

#endif // PHONOMETRICA_FUNC_LIST_HPP
