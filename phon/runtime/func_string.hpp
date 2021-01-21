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
 * Purpose: String builtin functions.                                                                                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FUNC_STRING_HPP
#define PHONOMETRICA_FUNC_STRING_HPP

#include <phon/runtime.hpp>

namespace phonometrica {

static Variant string_init(Runtime &, std::span<Variant>)
{
	return String();
}

static Variant string_get_field(Runtime &rt, std::span<Variant> args)
{
	auto &s = cast<String>(args[0]);
	auto &key = cast<String>(args[1]);
	if (key == rt.length_string) {
		return s.grapheme_count();
	}
	else if (key == "first") {
		return s.left(1);
	}
	else if (key == "last") {
		return s.right(1);
	}

	throw error("[Index error] String type has no member named \"%\"", key);
}

static Variant string_contains(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);

	return s1.contains(s2);
}

static Variant string_starts_with(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);

	return s1.starts_with(s2);
}

static Variant string_ends_with(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);

	return s1.ends_with(s2);
}

static Variant string_find1(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);

	return s1.find(s2);
}

static Variant string_find2(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);
	intptr_t i = cast<intptr_t>(args[2]);

	return s1.find(s2, i);
}

static Variant string_find_back1(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);

	return s1.rfind(s2);
}

static Variant string_find_back2(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);
	intptr_t i = cast<intptr_t>(args[2]);

	return s1.rfind(s2, i);
}

static Variant string_left(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto count = cast<intptr_t>(args[1]);

	return s1.left(count);
}

static Variant string_right(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto count = cast<intptr_t>(args[1]);

	return s1.right(count);
}

static Variant string_slice1(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto from = cast<intptr_t>(args[1]);

	return s1.mid(from);
}

static Variant string_slice2(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto from = cast<intptr_t>(args[1]);
	auto count = cast<intptr_t>(args[2]);

	return s1.mid(from, count);
}

static Variant string_count(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);

	return s1.count(s2);
}

static Variant string_to_upper(Runtime &, std::span<Variant> args)
{
	auto &s = cast<String>(args[0]);
	return s.to_upper();
}

static Variant string_to_lower(Runtime &, std::span<Variant> args)
{
	auto &s = cast<String>(args[0]);
	return s.to_lower();
}

static Variant string_reverse(Runtime &, std::span<Variant> args)
{
	auto &s = cast<String>(args[0]);
	s = s.reverse();

	return Variant();
}

static Variant string_is_empty(Runtime &, std::span<Variant> args)
{
	auto &s = cast<String>(args[0]);
	return s.empty();
}

static Variant string_char(Runtime &, std::span<Variant> args)
{
	auto &s = cast<String>(args[0]);
	intptr_t i = cast<intptr_t>(args[1]);

	return s.next_grapheme(i);
}

static Variant string_split(Runtime &rt, std::span<Variant> args)
{
	auto &s = cast<String>(args[0]);
	auto &delim = cast<String>(args[1]);
	Array<Variant> result;
	for (auto &p : s.split(delim)) {
		result.append(std::move(p));
	}

	return make_handle<List>(&rt, std::move(result));
}

static Variant string_append(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);
	s1.append(s2);

	return Variant();
}

static Variant string_prepend(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);
	s1.prepend(s2);

	return Variant();
}

static Variant string_insert(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	intptr_t pos = cast<intptr_t>(args[1]);
	auto &s2 = cast<String>(args[2]);
	s1.insert(pos, s2);

	return Variant();
}

static Variant string_trim(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	s1.trim();

	return Variant();
}

static Variant string_ltrim(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	s1.ltrim();

	return Variant();
}

static Variant string_rtrim(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	s1.rtrim();

	return Variant();
}

static Variant string_remove(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);
	s1.remove(s2);

	return Variant();
}

static Variant string_remove_first(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);
	s1.remove_first(s2);

	return Variant();
}

static Variant string_remove_last(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);
	s1.remove_last(s2);

	return Variant();
}

static Variant string_remove_at(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	intptr_t pos = cast<intptr_t>(args[1]);
	intptr_t count = cast<intptr_t>(args[2]);
	s1.remove(pos, count);

	return Variant();
}

static Variant string_replace(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);
	auto &s3 = cast<String>(args[2]);
	s1.replace(s2, s3);

	return Variant();
}

static Variant string_replace_first(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);
	auto &s3 = cast<String>(args[2]);
	s1.replace_first(s2, s3);

	return Variant();
}

static Variant string_replace_last(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);
	auto &s3 = cast<String>(args[2]);
	s1.replace_last(s2, s3);

	return Variant();
}

static Variant string_replace_at(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	intptr_t pos = cast<intptr_t>(args[1]);
	intptr_t count = cast<intptr_t>(args[2]);
	auto &s2 = cast<String>(args[0]);
	s1.replace(pos, count, s2);

	return Variant();
}

} // namespace phonometrica

#endif // PHONOMETRICA_FUNC_STRING_HPP
